#include "adx_file.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	// FILE* adxfile = fopen("B4VSTGUN.adx", "rb");
	FILE* adxfile = fopen("09_boss1_44k.adx", "rb");
	void* data = malloc(10 * 1024 * 1024);
	if (!(adxfile && data)) {
		return 1;
	}
	fread(data, 10 * 1024 * 1024, 1, adxfile);
	fclose(adxfile);

	ADXFileHead adx;
	adx_read_head(&adx, data);
	print_adx_head(&adx);
	adx_calc_len(&adx);

	free(data);
	return 0;
}
