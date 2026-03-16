#include <stdint.h>
#include "slime.h"

extern uint64_t worldseed;

// does what you think it does. slimes can spawn in a chunk if this returns 0
int checkSlimeChunk(int xPos, int zPos) {
	uint64_t seed = worldseed +
		(uint64_t)(int32_t)(xPos * xPos * 0x4C1906) + 
		(uint64_t)(int32_t)(xPos * 0x5AC0DB) +
		(uint64_t)(int32_t)(zPos * zPos) * 0x4307A7ULL +
		(uint64_t)(int32_t)(zPos * 0x5F24F);
	seed = ((seed ^ 0x5E434E432ULL) * 0x5DEECE66DULL + 0xB) & 0xFFFFFFFFFFFFULL;
	return (int)(seed >> 17) % 10;
}