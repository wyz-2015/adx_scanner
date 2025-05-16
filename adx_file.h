#ifndef _ADX_FILE_H
#define _ADX_FILE_H

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
/*
 * https://wiki.multimedia.cx/index.php/CRI_ADX_file
 *
Address	Size	Contains
========================
0x00 	2 	0x8000
0x02 	2 	data offset
0x04 	1 	format (3 for ADX)
0x05 	1 	block size (typically 18)
0x06 	1 	bits per sample? (4)
0x07 	1 	channel count
0x08 	4 	sample rate
0x0c 	4 	sample count
0x10 	2 	high-pass cutoff
0x12 	1 	loop data style ("type": 03, 04, or 05)
0x13 	1 	encryption flag (other flags?)

type 03 loop data
-----------------
0x14 	4 	unknown
0x18 	4 	loop flag
0x1c 	4 	loop start sample
0x20 	4 	loop start byte
0x24 	4 	loop end sample
0x28 	4 	loop end byte

type 04 loop data
-----------------
0x14 	16 	unknown
0x24 	4 	loop flag
0x28 	4 	loop start sample
0x2c 	4 	loop start byte
0x30 	4 	loop end sample
0x34 	4 	loop end byte
 *
 */

typedef struct ADXFileHead {
	uint16_t head;
	uint16_t dataOffset;
	uint8_t format;
	uint8_t blockSize;
	uint8_t bitsPerSample;
	uint8_t channelCount;
	uint32_t sampleRate;
	uint32_t sampleCount;
	uint16_t highPassCutoff;
	uint8_t loopDataStyle;
	uint8_t encryptionFlag;

	uint32_t loopFlag;
	uint32_t loopStartSample;
	uint32_t loopStartByte;
	uint32_t loopEndSample;
	uint32_t loopEndByte;
} ADXFileHead;

enum ENDIAN_CONVERT_MODE {
	BIG2LITTLE = 0,
	LITTLE2BIG = 1
};

long get_file_len(FILE* f);

bool adx_isHead(FILE* f);
bool adx_isDummyFrame(FILE* f);

void adx_head_endian_convert(const int mode, ADXFileHead* adxHead);
// void adx_read_head(ADXFileHead* adxHead, const void* dataBuffer);
void adx_read_head(ADXFileHead* adxHead, FILE* f);
// uint32_t adx_calc_len(const ADXFileHead* adxHead);
void fprint_adx_head(FILE* outFile, const ADXFileHead* adxHead);
#define print_adx_head(adxHead) fprint_adx_head(stdout, adxHead)
void dump_adx(FILE* inFile, const long adxFileStart, const long adxFileEnd, const char* outDir, const unsigned int fileCount);
void find_adx(FILE* inFile, FILE* logOutFile, const char* outDir);
// void find_adx2(FILE* inFile, FILE* logOutFile);

#endif
