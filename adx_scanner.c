#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void raise_error(const int errCode, const char* message, const char* funcName)
{
	if (*funcName) {
		fprintf(stderr, "函数“%s”出现错误：%s。返回码：%d。\n", funcName, message, errCode);
	} else {
		fprintf(stderr, "出现错误：%s。返回码：%d。\n", message, errCode);
	}
}

#define malloc_error() raise_error(1, "malloc函数分配内存失败", __func__);
#define fopen_error() raise_error(2, "文件打开失败", __func__);

#define mb2byte(mb) ((mb) * 1024 * 1024)

const uint32_t get_file_len(FILE* f)
{
	uint32_t posBackup, _len;
	posBackup = ftell(f);

	fseek(f, 0, 2);
	_len = ftell(f) + 1;

	fseek(f, posBackup, 0);

	return _len;
}

typedef struct HugeFileBuffer {
	FILE* file;
	void* blockBuffer;
	uint32_t bufferSize;	// Byte
	uint32_t maxBufferSize; // Byte
	uint32_t currentFileOffset;
} HugeFileBuffer;

void HugeFileBuffer_open(HugeFileBuffer* obj, const char* filePath)
{
	obj->file = fopen(filePath, "rb");
	if (not obj->file) {
		fopen_error();
	}
}

void HugeFileBuffer_close(HugeFileBuffer* obj)
{
	if (obj->blockBuffer) {
		free(obj->blockBuffer);
	}
	fclose(obj->file);

	free(obj);
	obj = NULL;
}

void HugeFileBuffer_init(HugeFileBuffer* obj)
{
	obj->file = NULL;
	obj->blockBuffer = NULL;
	obj->bufferSize = 0;
	obj->maxBufferSize = 0;
	obj->currentFileOffset = 0;
}

void HugeFileBuffer_setMaxBufferSize(HugeFileBuffer* obj, const uint32_t size)
{
	obj->maxBufferSize = size;
}

void HugeFileBuffer_readBlock(HugeFileBuffer* obj)
{
	if (obj->maxBufferSize == 0) {
		raise_error(3, "maxBufferSize不能为0！请检查是否忘记设置或设置失当。", __func__);
	}

	uint32_t howLong2End = (get_file_len(obj->file) - 1) - obj->currentFileOffset + 1; // 当前文件指针到文件末尾的距离，例如0-1，长为2.
	if (howLong2End > mb2byte(obj->maxBufferSize)) {				   // 以obj->maxBufferSize为单位分块读取，若剩余不足，则索要内存时也只索要一点。
		obj->bufferSize = obj->maxBufferSize;
	} else {
		obj->bufferSize = howLong2End;
	}

	if (obj->blockBuffer) { // 先释放上次分配的内存，再新分配本次使用的。若为NULL就是首次分配。
		free(obj->blockBuffer);
		obj->blockBuffer = NULL;
	}
	obj->blockBuffer = malloc(obj->bufferSize);
	if (not obj->blockBuffer) {
		malloc_error();
	}

	fread(obj->blockBuffer, obj->bufferSize, 1, obj->file);
	obj->currentFileOffset = ftell(obj->file);
}

int main(void)
{

	return 0;
}
