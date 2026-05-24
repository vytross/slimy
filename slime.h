#include <stdint.h>

static short circleTables[25][16][16][17][17];

int checkSlimeChunk(int, int);

void generateRegion(int*, int, int, struct chunkPos);
char** generateDensityGrid(int**, int, int);

int countSpawningSpaces(int**, struct chunkPos, struct chunkPos, struct blockPos*, int, int*);
int countCircleChunks(int**, struct chunkPos, struct chunkPos);

struct blockPos {
	int x, y, z;
};

struct chunkPos {
	int x, z;
};