#include "adx_file.h"
#include <stdio.h>
#include <string.h>
#include <iso646.h>

/*
void memread(void* dest, const void* src, size_t n, uint64_t* ptr)
{
	memcpy(dest, src + *ptr, n);
	*ptr += 1;
}

void read_adx_head(ADXFileHead* adxHead, const void* dataBuffer)
{
	uint64_t ptr = 0;
	memread(&(adxHead->head), dataBuffer, 2, &ptr);
	memread(&(adxHead->dataOffset), dataBuffer, 2, &ptr);
	memread(&(adxHead->format), dataBuffer, 1, &ptr);
	memread(&(adxHead->blockSize), dataBuffer, 1, &ptr);
	memread(&(adxHead->bitsPerSample), dataBuffer, 1, &ptr);
	memread(&(adxHead->channelCount), dataBuffer, 1, &ptr);
	memread(&(adxHead->sampleRate), dataBuffer, 4, &ptr);
	memread(&(adxHead->sampleCount), dataBuffer, 4, &ptr);
	memread(&(adxHead->highPassCutoff), dataBuffer, 2, &ptr);
	memread(&(adxHead->head), dataBuffer, 2, &ptr);
	memread(&(adxHead->loopDataStyle), dataBuffer, 1, &ptr);
	memread(&(adxHead->encryptionFlag), dataBuffer, 1, &ptr);
}
*/

/*
 * https://wiki.multimedia.cx/index.php/CRI_ADX_file
 *
Address Size    Contains
========================
0x00    2       0x8000
0x02    2       data offset
0x04    1       format (3 for ADX)
0x05    1       block size (typically 18)
0x06    1       bits per sample? (4)
0x07    1       channel count
0x08    4       sample rate
0x0c    4       sample count
0x10    2       high-pass cutoff
0x12    1       loop data style ("type": 03, 04, or 05)
0x13    1       encryption flag (other flags?)

type 03 loop data
-----------------
0x14    4       unknown
0x18    4       loop flag
0x1c    4       loop start sample
0x20    4       loop start byte
0x24    4       loop end sample
0x28    4       loop end byte

type 04 loop data
-----------------
0x14    16      unknown
0x24    4       loop flag
0x28    4       loop start sample
0x2c    4       loop start byte
0x30    4       loop end sample
0x34    4       loop end byte
 *
 */

void adx_read_head(ADXFileHead* adxHead, const void* dataBuffer) {
	memcpy(	&(adxHead->head),		dataBuffer + 0x00,	2	);
	memcpy(	&(adxHead->dataOffset),		dataBuffer + 0x02,	2	);
	memcpy(	&(adxHead->format),		dataBuffer + 0x04,	1	);
	memcpy(	&(adxHead->blockSize),		dataBuffer + 0x05,	1	);
	memcpy(	&(adxHead->bitsPerSample),	dataBuffer + 0x06,	1	);
	memcpy(	&(adxHead->channelCount),	dataBuffer + 0x07,	1	);
	memcpy(	&(adxHead->sampleRate),		dataBuffer + 0x08,	4	);
	memcpy(	&(adxHead->sampleCount),	dataBuffer + 0x0c,	4	);
	memcpy(	&(adxHead->highPassCutoff),	dataBuffer + 0x10,	2	);
	memcpy(	&(adxHead->loopDataStyle),	dataBuffer + 0x12,	1	);
	memcpy(	&(adxHead->encryptionFlag),	dataBuffer + 0x13,	1	);

	if (adxHead->loopDataStyle == 0x03) {
		memcpy(	&(adxHead->loopFlag),		dataBuffer + 0x18,	4	);
		memcpy(	&(adxHead->loopStartSample),	dataBuffer + 0x1c,	4	);
		memcpy(	&(adxHead->loopStartByte),	dataBuffer + 0x20,	4	);
		memcpy(	&(adxHead->loopEndSample),	dataBuffer + 0x24,	4	);
		memcpy(	&(adxHead->loopEndByte),	dataBuffer + 0x28,	4	);
	}
	else if (adxHead->loopDataStyle == 0x04){
		memcpy(	&(adxHead->loopFlag),		dataBuffer + 0x24,	4	);
		memcpy(	&(adxHead->loopStartSample),	dataBuffer + 0x28,	4	);
		memcpy(	&(adxHead->loopStartByte),	dataBuffer + 0x2c,	4	);
		memcpy(	&(adxHead->loopEndSample),	dataBuffer + 0x30,	4	);
		memcpy(	&(adxHead->loopEndByte),	dataBuffer + 0x34,	4	);
	}
}

void fprint_adx_head(FILE* outFile, const ADXFileHead* adxHead) {
	fprintf(outFile,
			"head: 0x%x\ndata offset: 0x%x\nformat: %u\nblock size: %u\nbits per sample: %u\nchannel count: %u\nsample rate: %u\nsample count: %u\nhigh-pass cutoff: %u\nloop data style: %u\nencryption flag: %u\n",
			adxHead->head,
			adxHead->dataOffset,
			adxHead->format,
			adxHead->blockSize,
			adxHead->bitsPerSample,
			adxHead->channelCount,
			adxHead->sampleRate,
			adxHead->sampleCount,
			adxHead->highPassCutoff,
			adxHead->loopDataStyle,
			adxHead->encryptionFlag
	       );
	if (adxHead->loopDataStyle == 0x03 or adxHead->loopDataStyle == 0x04) {
		fprintf(outFile,
				"loop flag:%u\nloop start sample:%u\nloop start style:%u\nloop end sample:%u\nloop end style:%u\n",
				adxHead->loopFlag,
				adxHead->loopStartSample,
				adxHead->loopStartByte,
				adxHead->loopEndSample,
				adxHead->loopEndByte
		);
	}
}

uint32_t adx_calc_len(const ADXFileHead* adxHead) {
	uint32_t len_head, len_data, len_dummyFrame;
	
	len_head = 0x04 + adxHead->dataOffset - 1; // 从文件0x00到正文前的部分
	len_data = adxHead->blockSize * adxHead->channelCount * adxHead->sampleCount;
	//len_dummyFrame = 
	printf("0x%x\n", len_data + len_head);

	return 0;
}
