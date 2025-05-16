#include "adx_file.h"
#include "usual_error.h"
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define mb2byte(n) ((n) * 1024 * 1024)

void print_help(const char* programName)
{
	printf("%s [参数]\n\t-i/--input_file\t传入文件 必需\n\t-o/--out_dir\t输出文件的目录 必需\n\n\t", programName);
}

int main(int argc, char** argv)
{
	char *outDir = "", *logFilePath = NULL, *inFilePath = NULL;
	size_t bufferSize = 0;

	int opt = 0, optIndex = 0;
	static struct option longOptions[] = {
		{ "out_dir", required_argument, NULL, 'o' },
		{ "log_file", optional_argument, NULL, 'l' },
		{ "buffer_size", optional_argument, NULL, 's' },
		{ "input_file", required_argument, NULL, 'i' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, no_argument, NULL, '\0' }
	};
	while ((opt = getopt_long(argc, argv, "i:o:l::s::h", longOptions, &optIndex)) != EOF) {
		switch (opt) {
		case 'o': {
			outDir = optarg;
			break;
		}
		case 'h': {
			print_help(argv[0]);
			return 0;
		}
		case 'l': {
			logFilePath = optarg;
			break;
		}
		case 'i': {
			inFilePath = optarg;
			break;
		}
		case 's': {
			int n = sscanf(optarg, "%lu", &bufferSize);
			if (n != 1) {
				fputs("-s选项参数读入失败\n", stderr);
				return 1;
			}
			break;
		}
		case '?': {
			fprintf(stderr, "未知选项：-%c\n", (char)optopt);
			break;
		}
		default: {
			print_help(argv[0]);
			return 1;
		}
		}
	}

	FILE *logOutFile, *inFile;
	void* buffer;

	if (logFilePath and *logFilePath) {
		logOutFile = fopen(logFilePath, "wt");
		if (not logOutFile) {
			fopen_error();
		}
		printf("信息将输出到文件：%s\n", logFilePath);
	} else {
		logOutFile = stdout;
		puts("信息将输出到stdout。");
	}

	if (inFilePath and *inFilePath) {
		inFile = fopen(inFilePath, "rb");
		if (not inFile) {
			fopen_error();
		}
		printf("读入文件：%s\n", inFilePath);

		if (bufferSize) {
			buffer = malloc(mb2byte(bufferSize));
			if (not buffer) {
				malloc_error();
			}
			if (setvbuf(inFile, buffer, _IOFBF, mb2byte(bufferSize)) != 0) {
				error(1, ENOMEM, "设定缓冲区失败\n");
			}
			printf("已设定缓冲区大小：%lu MiB, 合%lu Byte。\n", bufferSize, mb2byte(bufferSize));
		}
	}

	puts("预备工作成功完成");
	/* *************************************************** */
	// printf("%s: %p\n", __func__, inFile);
	find_adx(inFile, logOutFile, outDir);
	// find_adx2(inFile, logOutFile);
	/* *************************************************** */

	if (inFile) {
		fclose(inFile);
	}
	if (logFilePath) {
		fclose(logOutFile);
	}
	if (buffer) {
		free(buffer);
	}
	return 0;
}
