// calc is short for calculator btw. it's slang
#include "slime.h"
#include <stdio.h>

void generateCircleTables() {
	for (int y = 0; y < 25; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				for (int zChunk = -8; zChunk <= 8; zChunk++) {
					for (int xChunk = -8; xChunk <= 8; xChunk++) {
						circleTables[y][z][x][zChunk + 8][xChunk + 8] = 0;
						for (int zPos = 0; zPos < 16; zPos++) {
							for (int xPos = 0; xPos < 16; xPos++) {
								int distance = (xChunk * 16 + xPos - x) * (xChunk * 16 + xPos - x) + (zChunk * 16 + zPos - z) * (zChunk * 16 + zPos - z) + y * y;
								if (distance <= 16384 && distance > 576) {
									circleTables[y][z][x][zChunk + 8][xChunk + 8]++;
								}
							}
						}
					}
				}
			}
		}
	}
}

int countSpawningSpaces(int** region, struct chunkPos regionStart, struct chunkPos hotspot, struct blockPos* maxPos, int threshold, int* chunks) {
	// this preliminary check is super fast and filters out the vast majority of hotspots that aren't quite up to par
	*chunks = countCircleChunks(region, regionStart, hotspot);
	if (*chunks < threshold) return 0;

	unsigned short chunkArea[17][17];
	for (int zOffset = -8; zOffset <= 8; zOffset++) {
		for (int xOffset = -8; xOffset <= 8; xOffset++) {
			chunkArea[zOffset + 8][xOffset + 8] = 0;
			if (region[hotspot.z - regionStart.z + zOffset][(hotspot.x - regionStart.x + xOffset) >> 5] & (1 << ((hotspot.x - regionStart.x + xOffset) % 32)))
				chunkArea[zOffset + 8][xOffset + 8] = 0xFFFF;
		}
	}
	
	// this is maybe the slowest code of all time. gotta work on that
	int maxSpawningSpaces = 0;
	for (int y = 0; y < 25; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				int spawningSpaces = 0;
				for (int zOffset = -8; zOffset <= 8; zOffset++) {
					for (int xOffset = -8; xOffset <= 8; xOffset++) {
						spawningSpaces += (chunkArea[zOffset + 8][xOffset + 8] & circleTables[y][z][x][zOffset + 8][xOffset + 8]);
					}
				}
				if (spawningSpaces >= maxSpawningSpaces) {
					maxSpawningSpaces = spawningSpaces;
					maxPos->x = (hotspot.x * 16) + x;
					maxPos->y = y;
					maxPos->z = (hotspot.z * 16) + z;
				}
			}
		}
	}

	return maxSpawningSpaces;
}

int countCircleChunks(int** region, struct chunkPos regionStart, struct chunkPos hotspot) {
	int circleBitmap[17] = { 0xFE0, 0x3FF8, 0x7FFC, 0xFFFE, 0xFFFE, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0xFFFE, 0xFFFE, 0x7FFC, 0x3FF8, 0xFE0 };
	int chunkSum = 0;
	int x = hotspot.x - regionStart.x - 8;
	for (int zOffset = -8; zOffset <= 8; zOffset++) {
		int z = zOffset + hotspot.z - regionStart.z;
		int xSum = (region[z][x >> 5] & (circleBitmap[zOffset + 8] << (x % 32))) | ((x % 32 < 15) ? 0 : (region[z][(x >> 5) + 1] & (circleBitmap[zOffset + 8] >> (32 - (x % 32)))));
		for (; xSum; chunkSum++) xSum &= xSum - 1;
	}
	return chunkSum;
}
