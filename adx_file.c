#include "adx_file.h"
#include <arpa/inet.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usual_error.h"

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
 0x02    2       data offset (相对与0x04。即(0x04 + dataOffset)才是音频正文的起始位置)
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
 0x28    4       loop end byte (绝对位置)

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

long get_file_len(FILE* f)
{
	// printf("%s: %p\n", __func__, f);
	long posBackup = ftell(f);
	fseek(f, 0, 2);
	long _len = ftell(f);
	fseek(f, posBackup, 0);

	return _len;
}

void adx_head_endian_convert(const int mode, ADXFileHead* adxHead)
{ // 操作系统能正常读取的是小端序的数据，ADX文件为大端序格式。使用时需要作一些转化。
	uint16_t (*cov16)(uint16_t n);
	uint32_t (*cov32)(uint32_t n);
	if (mode == LITTLE2BIG) {
		cov16 = htons;
		cov32 = htonl;
	} else if (mode == BIG2LITTLE) {
		cov16 = ntohs;
		cov32 = ntohl;
	}
	// 这样的设计，允许一个函数皆可用于大转小与小转大。

	adxHead->head = cov16(adxHead->head);
	adxHead->dataOffset = cov16(adxHead->dataOffset);
	adxHead->sampleRate = cov32(adxHead->sampleRate);
	adxHead->sampleCount = cov32(adxHead->sampleCount);
	adxHead->highPassCutoff = cov16(adxHead->highPassCutoff);

	if (adxHead->loopDataStyle == 0x03 or adxHead->loopDataStyle == 0x04) {
		adxHead->loopFlag = cov32(adxHead->loopFlag);
		adxHead->loopStartSample = cov32(adxHead->loopStartSample);
		adxHead->loopStartByte = cov32(adxHead->loopStartByte);
		adxHead->loopEndSample = cov32(adxHead->loopEndSample);
		adxHead->loopEndByte = cov32(adxHead->loopEndByte);
	}
}

void adx_read_head(ADXFileHead* adxHead, FILE* f) // const void* dataBuffer)
{						  // 如果确定是adx文件的开头，则读取文件头信息
	long posBackup = ftell(f);
	void* dataBuffer = malloc(0x38); // 记载信息的文件头最长也就0x38 byte
	if (not dataBuffer) {
		malloc_error();
	}
	fread(dataBuffer, 0x38, 1, f);

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

	if (adxHead->loopDataStyle == 0x03 or adxHead->loopDataStyle == 0x04) {
		memcpy(	&(adxHead->loopFlag),		dataBuffer + 0x18,	4	);
		memcpy(	&(adxHead->loopStartSample),	dataBuffer + 0x1c,	4	);
		memcpy(	&(adxHead->loopStartByte),	dataBuffer + 0x20,	4	);
		memcpy(	&(adxHead->loopEndSample),	dataBuffer + 0x24,	4	);
		memcpy(	&(adxHead->loopEndByte),	dataBuffer + 0x28,	4	);
	}
	else if (adxHead->loopDataStyle == 0x04) {
		memcpy(	&(adxHead->loopFlag),		dataBuffer + 0x24,	4	);
		memcpy(	&(adxHead->loopStartSample),	dataBuffer + 0x28,	4	);
		memcpy(	&(adxHead->loopStartByte),	dataBuffer + 0x2c,	4	);
		memcpy(	&(adxHead->loopEndSample),	dataBuffer + 0x30,	4	);
		memcpy(	&(adxHead->loopEndByte),	dataBuffer + 0x34,	4	);
	}

	adx_head_endian_convert(BIG2LITTLE, adxHead);

	fseek(f, posBackup, 0);
	free(dataBuffer);
}

void fprint_adx_head(FILE* outFile, const ADXFileHead* adxHead)
{ // 打印文件头信息，可选向文件打印。另有宏定义，名字无“f”，默认向stdout打印。
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
			adxHead->encryptionFlag);
	if (adxHead->loopDataStyle == 0x03 or adxHead->loopDataStyle == 0x04) {
		fprintf(outFile,
				"loop flag: %u\nloop start sample: %u\nloop start byte: 0x%x\nloop end sample: %u\nloop end byte: 0x%x\n",
				adxHead->loopFlag,
				adxHead->loopStartSample,
				adxHead->loopStartByte,
				adxHead->loopEndSample,
				adxHead->loopEndByte);
	}

	fputc('\n', outFile);
}

/*
uint32_t adx_calc_len(const ADXFileHead* adxHead)
{ // 此函数必须弃用：计算思路由AI提供，然而经验证根本不科学。
	uint32_t len_head, len_data, len_dummyFrame;

	len_head = 0x04 + adxHead->dataOffset - 1; // 从文件0x00到正文前的部分
	len_data = adxHead->blockSize * adxHead->channelCount * adxHead->sampleCount;
	// len_dummyFrame =
	printf("0x%x\n", len_data + len_head);

	return 114514;
}
*/

bool adx_isHead(FILE* f)
{ // 当前文件指针是否正指向一个adx文件的开头
	long posBackup = ftell(f);

	uint16_t head, dataOffset;
	fread(&head, 0x2, 1, f);
	fread(&dataOffset, 0x2, 1, f);
	head = ntohs(head);
	dataOffset = ntohs(dataOffset);

	if (head != 0x8000 or dataOffset == 0x0000) {
		fseek(f, posBackup, 0); // 文件指针复位。
		return false;
	}

	fseek(f, dataOffset - 0x6, 1);
	char sign[7] = { 0 }; // 验证adx文件签名“(c)CRI”。
	fread(sign, 6, 1, f);

	fseek(f, posBackup, 0); // 文件指针复位。

	return strcmp(sign, "(c)CRI") == 0;
}

bool adx_isDummyFrame(FILE* f)
{ // 和判断头不一样，判断是否为结尾的虚拟帧。
	long posBackup = ftell(f);

	uint16_t head, dataOffset;
	fread(&head, 0x2, 1, f);
	fread(&dataOffset, 0x2, 1, f);
	head = ntohs(head);
	dataOffset = ntohs(dataOffset);

	if (head != 0x8001 or dataOffset == 0x0000) {
		fseek(f, posBackup, 0); // 文件指针复位。
		return false;
	}

	fseek(f, dataOffset - 0x1, 1);
	uint8_t byte1, byte2;
	fread(&byte1, 1, 1, f);
	uint8_t c = fread(&byte2, 1, 1, f);

	fseek(f, posBackup, 0); // 文件指针复位。

	if (/*c != 1 or */byte1 == 0x0) {
		return true;
	}

	return false;
}

void dump_adx(FILE* inFile, const long adxFileStart, const long adxFileEnd, const char* outDir, const unsigned int fileCount)
{ // 将adx文件拷贝出来，自动命名为00n.adx
	char path[256];
	sprintf(path, "%s/%03u.adx", outDir, fileCount);

	long fileLen = adxFileEnd - adxFileStart + 1;
	printf("Start: 0x%08lx, End: 0x%08lx\n文件长度：%lu\n", adxFileStart, adxFileEnd, fileLen);
	void* adxFileData = malloc(fileLen);
	if (not adxFileData) {
		malloc_error();
	}

	long posBackup = ftell(inFile);
	fseek(inFile, adxFileStart, 0);
	fread(adxFileData, fileLen, 1, inFile);
	fseek(inFile, posBackup, 0);

	FILE* outFile = fopen(path, "wb");
	if (not outFile) {
		fopen_error();
	}
	fwrite(adxFileData, fileLen, 1, outFile);

	free(adxFileData);
	fclose(outFile);
}

void find_adx(FILE* inFile, FILE* logOutFile, const char* outDir)
{
	// printf("%s: %p\n", __func__, inFile);
	unsigned int fileCount = 0, adxHeadCount = 0;
	const long fileEndPos = get_file_len(inFile);
	long adxFileStart, adxFileEnd, mayBeEndDummyFrame;
	bool foundEndDummyFrame = false, recording = false;
	ADXFileHead adxHead;

	while (ftell(inFile) <= fileEndPos) {
		// printf("0x%lx\n", ftell(inFile));

		if (adx_isHead(inFile)) {
			/* 发现头，则读取头信息 */
			adx_read_head(&adxHead, inFile);
			if (recording) {
				fseek(inFile, mayBeEndDummyFrame, 0);
				foundEndDummyFrame = true;
				continue;
			} else {
				adxFileStart = ftell(inFile);
				recording = true;

				/* 如果能通过文件头信息直接定位到文件尾，则直接诱导到结尾虚拟帧，随流程dump出来 */
				/* 废弃：一旦出现不符常规格式的adx文件，则流程只会大乱 */
				if (adxHead.loopDataStyle == 0x03 or adxHead.loopDataStyle == 0x04) {
					if (adxHead.loopFlag == 1) {
						fseek(inFile, adxHead.loopEndByte, 1);
						foundEndDummyFrame = true;
						continue;
					}
				}
			}

			/*
			// recording = true;
			adxHeadCount += 1;
			printf("ADXHead: 0x%08lx\n", ftell(inFile));
			if (adxHeadCount == 2) {
			fseek(inFile, mayBeEndDummyFrame, 0);
			foundEndDummyFrame = true;
			adxHeadCount = 0;
			continue;
			}
			*/
			/* 从无开始，读取到第1个ADXHead，计数器+1，流程正常
			 * 一旦发现计数器为2，即当前已经找到了2个ADXHead，则最后找到的疑似虚拟帧的一定是一个真的末尾虚拟帧
			 * 此后故意跳转到最后发现的虚拟帧处，并告知这个虚拟帧是末尾虚拟帧。随流程保存
			 */

		} else if (adx_isDummyFrame(inFile)) {
			if (recording) {
				mayBeEndDummyFrame = ftell(inFile);
				// printf("疑似虚拟帧：0x%lx\n", mayBeEndDummyFrame);

				if (foundEndDummyFrame) { // 文件保存
					fseek(inFile, 2, 1);
					uint16_t offset;
					fread(&offset, 2, 1, inFile);
					offset = ntohs(offset);

					fseek(inFile, offset - 0x1, 1);
					adxFileEnd = ftell(inFile);

					fprintf(logOutFile, "找到第%u个.adx文件：在传入文件的[0x%08lx, 0x%08lx]处\n", fileCount, adxFileStart, adxFileEnd);
					fprint_adx_head(logOutFile, &adxHead);
					dump_adx(inFile, adxFileStart, adxFileEnd, outDir, fileCount);

					fileCount += 1;
					foundEndDummyFrame = false;
					// adxHeadCount = 0;
					recording = false;
					/*
					   adxFileStart = 0;
					   adxFileEnd = 0;
					   mayBeEndDummyFrame = 0;
					*/
				}
			}
		}
		fseek(inFile, 1, 1);
	}
	/* 收尾时，保存下最后一份文件 */
	if (recording) {
		fseek(inFile, mayBeEndDummyFrame + 0x02, 0);
		uint16_t offset;
		fread(&offset, 2, 1, inFile);
		offset = ntohs(offset);

		fseek(inFile, offset - 0x1, 1);
		adxFileEnd = ftell(inFile);

		fprintf(logOutFile, "找到第%u个.adx文件：在传入文件的[0x%08lx, 0x%08lx]处\n", fileCount, adxFileStart, adxFileEnd);
		fprint_adx_head(logOutFile, &adxHead);
		dump_adx(inFile, adxFileStart, adxFileEnd, outDir, fileCount);
	}
}

/*
void find_adx2(FILE* inFile, FILE* logOutFile)
{
	unsigned int fileCount = 0;
	const long fileEndPos = get_file_len(inFile);
	ADXFileHead adxHead;
	while (ftell(inFile) <= fileEndPos) {
		if (adx_isHead(inFile)) {
			adx_read_head(&adxHead, inFile);
			fprintf(logOutFile, "找到第%u个 疑似 .adx文件：在传入文件的0x%08lx处\n", fileCount, ftell(inFile));
			fprint_adx_head(logOutFile, &adxHead);
			fileCount += 1;
		}
		fseek(inFile, 1, 1);
	}
}
*/
