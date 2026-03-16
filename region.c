// no relation to minecraft regions btw

#include <stdlib.h>
#include "slime.h"

// look into sending this task to the gpu
void generateRegion(int** region, int xSize, int zSize, int xStart, int zStart) {
	for (int z = 0; z < zSize; z++, region++) {
		int* slice = *region;
		for (int x = 0; x < xSize; x++, slice++) for (int i = 0; i < 32; i++) *slice |= !checkSlimeChunk(xStart + (x << 5) + i, zStart + z) << i;
	}
}

// leetcode ass function. also maybe worth sending to gpu. would be a good bit harder but like, look at all these for loops
char** generateDensityGrid(int** region, int xSize, int zSize) {
	char** xAxisDensity = calloc(zSize, sizeof(char*));

	for (int z = 0; z < zSize; z++) {
		xAxisDensity[z] = calloc((xSize << 5) - 16, sizeof(char));
		char* val = xAxisDensity[z];

		// if i'm clever i might be able to fit the bottom loop into here for even faster generation
		for (int x = 0; x < (xSize << 5) - 16; x++, val++) {
			int chunkCount = (region[z][x >> 5] & (0x1FFFF << (x % 32))) | ((x % 32 < 15) ? 0 : (region[z][(x >> 5) + 1] & (0x1FFFF >> (32 - (x % 32)))));
			for (; chunkCount; (*val)++) chunkCount &= (chunkCount - 1);
		}
	}

	char** density = calloc(zSize - 16, sizeof(char*));
	for (int z = 0; z < zSize - 16; z++) density[z] = calloc((xSize << 5) - 16, sizeof(char));

	char* initial = density[0];
	
	for (int x = 0; x < ((xSize << 5) - 16); x++) {
		for (int i = 0; i < 18; i++) initial[x] += xAxisDensity[i][x];
		for (int z = 1; z < (zSize - 16); z++) density[z][x] = density[z - 1][x] - xAxisDensity[z - 1][x] + xAxisDensity[z + 16][x];
	}

	for (int z = 0; z < zSize; z++) free(xAxisDensity[z]);
	free(xAxisDensity);

	return density;
}