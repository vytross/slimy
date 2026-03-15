#include <stdint.h>
#include "slime.h"

extern uint64_t worldseed;

// does what you think it does. slimes can spawn in a chunk if this returns 0
int checkSlimeChunk(int xPos, int zPos) {
	uint64_t seed = worldseed +
		(uint64_t)(int32_t)(xPos * xPos * 0x4c1906u) +
		(uint64_t)(int32_t)(xPos * 0x5ac0dbu) +
		(uint64_t)(int32_t)(zPos * zPos) * 0x4307a7ull +
		(uint64_t)(int32_t)(zPos * 0x5f24fu) ^ 0x3ad8025full;
	seed = (seed ^ 0x5deece66dull) & 0xffffffffffffull;
	seed = (seed * 0x5deece66dull + 0xb) & 0xffffffffffffull;
	return (int)(seed >> 17) % 10;
}