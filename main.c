#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "slime.h"

uint64_t worldseed;

int countSlimeChunks(int, int);

main() {

	if (INT_MAX != 0x7fffffff) return 32767; // i'm not making this compatible with whatever the fuck machine you're on

	const int threshold, range;

	// this is really terrible code and is very easy to stack overflow and corrupt everything. don't do this
	printf("worldseed: ");
	scanf("%llu", &worldseed);

	printf("\nchunk threshold: ");
	scanf("%d", &threshold);

	printf("\nchunk range: ");
	scanf("%d", &range);	

	int** region = calloc(range * 2, sizeof(int*));
	for (int i = 0; i < range * 2; i++) region[i] = calloc((range >> 4) + 1, sizeof(int));
	generateRegion(region, (range >> 4) + 1, range * 2, -range, -range);

	for (int zCenter = -range+7; zCenter < range-7; zCenter++) {
		for (int xCenter = -range+7; xCenter < range-7; xCenter++) {
			int chunks = 0;
			if (range > 500) chunks = countSlimeChunks(xCenter, zCenter); // arbitrary cutoff, mostly for testing
			if (range <= 500) {
				for (int x = xCenter - 7; x < xCenter + 7; x++) {
					for (int z = zCenter - 7; z < zCenter + 7; z++) {
						// todo make this less bad
						if (((x - xCenter + 0.5) * (x - xCenter + 0.5)) + ((z - zCenter + 0.5) * (z - zCenter + 0.5)) <= 64
							&& (region[z + range][(x + range) >> 5] & (1 << (31 - (x + range) % 32)))) chunks++;
					}
				}
			}
			if (chunks >= threshold) printf("%d chunks at (%d, %d)\n", chunks, xCenter * 16, zCenter * 16);
		}
	}
}

int countSlimeChunks(int xCenter, int zCenter) {
	int chunks = 0;
	for (int x = xCenter - 7; x < xCenter + 7; x++) {
		for (int z = zCenter - 7; z < zCenter + 7; z++) {
			if (((x - xCenter + 0.5) * (x - xCenter + 0.5)) + ((z - zCenter + 0.5) * (z - zCenter + 0.5)) <= 64 && !checkSlimeChunk(x, z)) {
				chunks++;
			}
		}
	}
	return chunks;
}