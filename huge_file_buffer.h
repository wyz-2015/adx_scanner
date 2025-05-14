#ifndef _HUGE_FILE_BUFFER_H
#define _HUGE_FILE_BUFFER_H

#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define mb2byte(mb) ((mb) * 1024 * 1024)

const uint32_t get_file_len(FILE* f);

typedef struct HugeFileBuffer {
	FILE* file;
	void* blockBuffer;
	uint32_t bufferSize;	// Byte
	uint32_t maxBufferSize; // Byte
	uint32_t currentFileOffset;
} HugeFileBuffer;

void HugeFileBuffer_open(HugeFileBuffer* obj, const char* filePath);
void HugeFileBuffer_close(HugeFileBuffer* obj);
void HugeFileBuffer_init(HugeFileBuffer* obj);
void HugeFileBuffer_setMaxBufferSize(HugeFileBuffer* obj, const uint32_t size);
void HugeFileBuffer_readBlock(HugeFileBuffer* obj);

#endif
