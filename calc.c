// calc is short for calculator btw. it's slang
#include "slime.h"

int chunkCircleCount(int** region, int xPos, int zPos, int xRangeStart, int zRangeStart) {
	int circleBitmap[17] = { 0xFE0, 0x3FF8, 0x7FFC, 0xFFFE, 0xFFFE, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0xFFFE, 0xFFFE, 0x7FFC, 0x3FF8, 0xFE0 };
	int chunkSum = 0;
	int x = xPos - xRangeStart - 8;
	for (int z = -8; z <= 8; z++) {
		int xSum = (region[z + zPos - zRangeStart][x >> 5] & (circleBitmap[z + 8] << (x % 32))) | ((x % 32 < 15) ? 0 : (region[z + zPos - zRangeStart][(x >> 5) + 1] & (circleBitmap[z + 8] >> (32 - (x % 32)))));
		for (; xSum; chunkSum++) xSum &= xSum - 1;
	}
	return chunkSum;
}