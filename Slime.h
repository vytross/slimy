#include <stdint.h>
#include <CL/cl.h>

static _Bool useGPU = 1;

int checkSlimeChunk(int, int);
cl_int generateCircleTables();
void generateCircleTablesOld();
int hotspotMaxSpawningSpaces(int*, int, struct chunkPos*, struct chunkPos*, struct blockPos*, int, int*);

void generateRegion(int*, int, int, struct chunkPos*, cl_mem*);
void generateDensityGrid(int*, int, int, char*, cl_mem*);

int checkSlimeChunkTest(int, int);

struct blockPos {
	int x, y, z;
};

struct chunkPos {
	int x, z;
};


struct chunkPos* generateHotspotIndexList(int*, int, int, int*, int);