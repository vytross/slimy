// no relation to minecraft regions btw

#include <stdlib.h>
#include "slime.h"

// look into sending this task to the gpu
void generateRegion(int** region, int xSize, int zSize, int xStart, int zStart) {
	for (int z = 0; z < zSize; z++) {
		int* slice = *region;
		for (int x = 0; x < xSize; x++) {
			// this might be more efficient little-endian
			int i = 32;
			while (i--) *slice |= !checkSlimeChunk(xStart + (x<<5) + 31 - i, zStart + z) << i;
			slice++;
		}
		region++;
	}
	return;
}