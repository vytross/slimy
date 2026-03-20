#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
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

	// generates a gigantic 2d integer array (that's secretly a bitmap in disguise) to store all the slime chunk data
	int** region = calloc(range * 2, sizeof(int*));
	for (int i = 0; i < range * 2; i++) region[i] = calloc((range >> 4) + 1, sizeof(int));
	generateRegion(region, (range >> 4) + 1, range * 2, -range, -range);

	// int clustercount = 0;

	clock_t start, end;
	start = clock();

	// the better algorithm (in progress). range cutoff is arbitrary but i know from testing that 500 and 501 give the same results
	if (range <= 500) {
		char** densityGrid = generateDensityGrid(region, (range >> 4) + 1, range * 2);
		for (int zCenter = -range + 8; zCenter < range - 8; zCenter++) {
			for (int xCenter = -range + 8; xCenter < range - 8; xCenter++) {
				// it's way faster to calculate a square around each chunk than a circle. if the square doesn't even have enough don't bother
				if (densityGrid[zCenter + range - 8][xCenter + range - 8] > threshold) {
					int chunks = chunkCircleCount(region, xCenter, zCenter, -range, -range);
					if (chunks >= threshold) {
						printf("%d chunks at (%d, %d)\n", chunks, xCenter * 16, zCenter * 16); // clustercount++;
					}
				}
			}
		}
	}
	// the standard approach, i keep it around for time comparison. mine wins :-)
	if (range > 500) {
		for (int zCenter = -range + 8; zCenter < range - 8; zCenter++) {
			for (int xCenter = -range + 8; xCenter < range - 8; xCenter++) {
				int chunks = countSlimeChunks(xCenter, zCenter); 
				if (chunks >= threshold) {
					printf("%d chunks at (%d, %d)\n", chunks, xCenter * 16, zCenter * 16); // clustercount++;
				}
			}
		}
	}
	end = clock();
	printf("Time taken: %f", (double)(end - start) / CLOCKS_PER_SEC);
	// printf("cluster count %d\n", clustercount);
}

int countSlimeChunks(int xCenter, int zCenter) {
	int chunks = 0;
	for (int x = xCenter - 8; x <= xCenter + 8; x++) {
		for (int z = zCenter - 8; z < zCenter + 8; z++) {
			if (((x - xCenter) * (x - xCenter)) + ((z - zCenter) * (z - zCenter)) <= 75 && !checkSlimeChunk(x, z)) {
				chunks++;
			}
		}
	}
	return chunks;
}