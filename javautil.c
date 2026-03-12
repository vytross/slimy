#include <stdbool.h>
#include <stdint.h>
#include "slime.h"

// does what you think it does
bool isSlimeChunk(int xPos, int zPos, uint64_t worldseed) {
	uint64_t seed = worldseed +
		(uint64_t)(int32_t)(xPos * xPos * 0x4c1906u) +
		(uint64_t)(int32_t)(xPos * 0x5ac0dbu) +
		(uint64_t)(int32_t)(zPos * zPos) * 0x4307a7ull +
		(uint64_t)(int32_t)(zPos * 0x5f24fu) ^ 0x3ad8025full;
	seed = (seed ^ 0x5deece66d) & 0xffffffffffff;
	seed = (seed * 0x5deece66d + 0xb) & 0xffffffffffff;
	return ((int)(seed >> 17) % 10) == 0;
}