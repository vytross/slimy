#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "slime.h"

int countSlimeChunks(int, int, int64_t);

main() {
	uint64_t worldseed;
	int threshold, range;

	// this is really terrible code and is very easy to stack overflow and corrupt everything. don't do this
	printf("worldseed: ");
	scanf("%llu", &worldseed);

	printf("\nchunk threshold: ");
	scanf("%d", &threshold);

	printf("\nchunk range: ");
	scanf("%d", &range);	

	for (int zCenter = -range; zCenter < range; zCenter++) {
		for (int xCenter = -range; xCenter < range; xCenter++) {
			int chunks = countSlimeChunks(xCenter, zCenter, worldseed);
			if (chunks >= threshold) {
				printf("%d chunks at (%d, %d)\n", chunks, xCenter * 16, zCenter * 16);
			}
		}
	}
}

int countSlimeChunks(int xCenter, int zCenter, uint64_t worldseed) {
	int chunks = 0;
	for (int x = xCenter - 7; x < xCenter + 7; x++) {
		for (int z = zCenter - 7; z < zCenter + 7; z++) {
			// todo make this less bad
			if (((x - xCenter + 0.5) * (x - xCenter + 0.5)) + ((z - zCenter + 0.5) * (z - zCenter + 0.5)) <= 64 && isSlimeChunk(x, z, worldseed)) {
				chunks++;
			}
		}
	}
	return chunks;
}