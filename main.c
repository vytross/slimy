#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "slime.h"

uint64_t worldseed;

main() {
	if (INT_MAX != 0x7FFFFFFF || SHRT_MAX != 0x7FFF) return 32767; // i'm not making this compatible with whatever the fuck machine you're on

	const int regionSize = 512;

	clock_t start2, end2;
	start2 = clock();
	generateCircleTables();
	end2 = clock();
	printf("circle table generation takes %f s\n", (double)(end2 - start2) / CLOCKS_PER_SEC);

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
	struct chunkPos regionStart = { -range, -range };
	generateRegion(region, (range >> 4) + 1, range * 2, regionStart);

	clock_t start, end;
	start = clock();

	if (range <= 10000) {
		// it's way faster to calculate a square around each chunk than a circle. if the square doesn't even have enough don't bother
		char** densityGrid = generateDensityGrid(region, (range >> 4) + 1, range * 2);
		for (int zPos = 0; zPos < (2 * range) - 16; zPos++) {
			for (int xPos = 0; xPos < (2 * range) - 16; xPos++) {
				if (densityGrid[zPos][xPos] >= threshold) {
					struct chunkPos hotspot = { xPos - range + 8, zPos - range + 8 };
					struct blockPos hotspotMaxBlock = { 0, 0, 0 };
					int chunks = 0;
					int spawningSpaces = countSpawningSpaces(region, regionStart, hotspot, &hotspotMaxBlock, threshold, &chunks);

					if (spawningSpaces >= threshold * 256) {
						printf("%d spawning spaces from %d chunks at (%d, %d, %d)\n", spawningSpaces, chunks, hotspotMaxBlock.x, hotspotMaxBlock.y, hotspotMaxBlock.z);
					}
				}
			}
		}
	}
	end = clock();
	printf("Time taken: %f", (double)(end - start) / CLOCKS_PER_SEC);
}