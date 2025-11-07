#include "adx_file.h"
#include "usual_error.h"
#include <errno.h>
#include <error.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>

#define mb2byte(n) ((n) * 1024 * 1024)

void print_help(const char* programName)
{
	printf("%s [参数]\n\t-i/--input_file\t传入文件 必需\n\t-o/--out_dir\t输出文件的目录 必需\n\n\t", programName);
}

typedef struct Arguments {
	char* inFilePath;
	char* outDir;
	size_t bufferSize;
	char* logFilePath;
} Arguments;

static const struct argp_option options[] = {
	{ "input_file", 'i', "FILE", 0, "输入文件路径" },
	{ "out_dir", 'o', "DIR", 0, "输出文件目录" },
	{ "log_file", 'l', "FILE", OPTION_ARG_OPTIONAL, "输出记录文件的路径" },
	{ "buffer_size", 's', "SIZE", OPTION_ARG_OPTIONAL, "文件IO中缓冲区的大小(单位MB)。若要处理大体积文件，可以考虑设置这个参数" },
	{ 0 }
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
	Arguments* arguments = state->input;
	
	switch(key){
		case 'i':
			{
				arguments->inFilePath = arg;
				break;
			}
		case 'o':
			{
				arguments->outDir = arg;
				break;
			}
		case 'l':
			{
				arguments->logFilePath = arg;
				break;
			}
		case 's':
			{
				if (arg == NULL or sscanf(arg, "%lu", &(arguments->bufferSize)) != 1) {
					error(EIO, EIO, "读取 -s / --buffer_size 的参数“%s”失败", arg);
				}
				if(arguments->bufferSize == 0) {
					error(0, EINVAL, "不允许设置缓冲区大小为0，程序将维持默认设置执行");
				}
				break;
			}
		case ARGP_KEY_ARG:
			{
				argp_usage(state);
				break;
			}
		/*
		case ARGP_KEY_END:
			{
				if (state->arg_num < 1) {
					error(0, EINVAL, "疑似根本未传递任何参数\n");
					argp_usage(state);
					
					return ARGP_ERR_UNKNOWN;
				}
				break;
			}
		*/
		default:
			{
				return ARGP_ERR_UNKNOWN;
			}
	}
	return 0;
}

static struct argp argp = {options, parse_opt, NULL, NULL};

int main(int argc, char** argv)
{
	Arguments args = { NULL, "", 0, NULL };
	argp_parse(&argp, argc, argv, 0, 0, &args);

	FILE *logOutFile = NULL, *inFile = NULL;
	void* buffer = NULL;

	if (args.logFilePath) {
		if (*(args.logFilePath)) {
			logOutFile = fopen(args.logFilePath, "wt");
			if (not logOutFile) {
				fopen_error();
			}
			printf("信息将输出到文件：%s\n", args.logFilePath);
		}
	} else {
		logOutFile = stdout;
		puts("信息将输出到stdout");
	}

	if (args.inFilePath) {
		if (*(args.inFilePath)) {
			inFile = fopen(args.inFilePath, "rb");
			if (not inFile) {
				fopen_error();
			}
			printf("读入文件：%s\n", args.inFilePath);
		}

		if (args.bufferSize > 0) {
			const size_t bufferSize_byte = mb2byte(args.bufferSize);
			buffer = malloc(bufferSize_byte);
			if (not buffer) {
				malloc_error();
			}
			if (setvbuf(inFile, buffer, _IOFBF, bufferSize_byte) != 0) {
				error(ENOMEM, ENOMEM, "设定缓冲区失败");
			}
			printf("已设定缓冲区大小：%lu MiB, 合%lu Byte。\n", args.bufferSize, bufferSize_byte);
		}
	} else {
		error(EINVAL, EINVAL, "疑似未传递 -i / --input_file 参数！");
	}

	if(not args.outDir){
		error(EINVAL, EINVAL, "疑似未传递 -o / --out_dir 参数！");
	}

	puts("预备工作成功完成");
	/* *************************************************** */
	// printf("%s: %p\n", __func__, inFile);
	find_adx(inFile, logOutFile, args.outDir);
	// find_adx2(inFile, logOutFile);
	/* *************************************************** */

	if (inFile) {
		fclose(inFile);
	}
	if (logOutFile and logOutFile != stdout) {
		fclose(logOutFile);
	}
	if (buffer) {
		free(buffer);
	}
	return 0;
}
