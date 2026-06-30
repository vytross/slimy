#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <CL/cl.h>

#include "Slime.h"
#include "GPU/GPU.h"

#define REGION_SIZE 8176 // TODO fine tune this. hardware dependent

uint64_t worldseed;

void worldSearch(uint64_t, int, int);
void regionSearch(uint64_t, int, int*, char**, struct chunkPos);

main() {
	extern _Bool useGPU;
	cl_int err = findGPU();
	if (err != CL_SUCCESS) useGPU = 0;

	//useGPU = 0;

	clock_t start1, end1;
	start1 = clock();

	generateCircleTables(useGPU);
	//generateCircleTablesOld();

	end1 = clock();
	
	printf("Circle table generation takes %f s\n", (double)(end1 - start1) / CLOCKS_PER_SEC);

	if (useGPU) buildKernels();

	int threshold, searchDistance;

	// this is really terrible code and is very easy to stack overflow and corrupt everything.
	// don't do this
	printf("Worldseed: ");
	scanf("%llu", &worldseed);

	printf("Desired spawning spaces: ");
	scanf("%d", &threshold);
	//threshold = 12000;

	printf("Search distance (in chunks): ");
	scanf("%d", &searchDistance);
	//searchDistance = 12000;

	clock_t start3, end3;
	start3 = clock();

	worldSearch(worldseed, threshold, searchDistance);

	end3 = clock();

	printf("Search time: %f s\n", (double)(end3 - start3) / CLOCKS_PER_SEC);
}

void worldSearch(uint64_t worldseed, int threshold, int searchDistance) {
	int regionDataZSize = REGION_SIZE + 16;
	int regionDataXSize = ((regionDataZSize - 1) >> 5) + 1;

	//int regionDataTemplate[(REGION_SIZE + 16) * (((REGION_SIZE + 15) >> 5) + 1)];
	int* regionDataTemplate = (int*)malloc(regionDataZSize * regionDataXSize * sizeof(int));

	//char** densityGridTemplate = malloc((regionDataZSize - 16) * sizeof(char*));
	//for (int j = 0; j < regionDataZSize - 16; j++) densityGridTemplate[j] = malloc(((regionDataXSize << 5) - 16) * sizeof(char));
	char* densityGridTemplate = malloc(REGION_SIZE * REGION_SIZE * sizeof(char));

	int ringCount;

	if (searchDistance <= (REGION_SIZE / 2)) ringCount = 0;
	else ringCount = (int)((searchDistance - (REGION_SIZE / 2)) / REGION_SIZE) + 1;

	struct chunkPos regionZeroStart = { -(REGION_SIZE / 2), -(REGION_SIZE / 2) };
	regionSearch(worldseed, threshold, regionDataTemplate, densityGridTemplate, regionZeroStart);

	for (int ring = 1; ring <= ringCount; ring++) {
		// set up so in the future this can maybe be multithreaded. idk how to do that yet
		for (int regionXCount = 0; regionXCount < (2 * ring); regionXCount++) {
			struct chunkPos regionStart = { regionZeroStart.x - (REGION_SIZE * ring) + (REGION_SIZE * regionXCount), regionZeroStart.z - (REGION_SIZE * ring) };
			regionSearch(worldseed, threshold, regionDataTemplate, densityGridTemplate, regionStart);

			struct chunkPos regionStartOpposite = { -(regionStart.x) - REGION_SIZE, -(regionStart.z) - REGION_SIZE };
			regionSearch(worldseed, threshold, regionDataTemplate, densityGridTemplate, regionStartOpposite);
		}

		for (int regionZCount = 0; regionZCount < (2 * ring); regionZCount++) {
			struct chunkPos regionStart = { regionZeroStart.x - (REGION_SIZE * ring), regionZeroStart.z - (REGION_SIZE * ring) + (REGION_SIZE * (regionZCount + 1)) };
			regionSearch(worldseed, threshold, regionDataTemplate, densityGridTemplate, regionStart);

			struct chunkPos regionStartOpposite = { -(regionStart.x) - REGION_SIZE, -(regionStart.z) - REGION_SIZE };
			regionSearch(worldseed, threshold, regionDataTemplate, densityGridTemplate, regionStartOpposite);
		}
	}
}

void regionSearch(uint64_t worldseed, int threshold, int* regionData, char* densityGrid, struct chunkPos regionStart) {
	int regionDataZSize = REGION_SIZE + 16;
	int regionDataXSize = ((regionDataZSize - 1) >> 5) + 1;
	struct chunkPos regionDataStart = { regionStart.x - 8, regionStart.z - 8 };
	cl_mem regionGPUBuffer = NULL;
	if (useGPU) regionGPUBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * regionDataXSize * regionDataZSize, NULL, NULL);
	generateRegion(regionData, regionDataXSize, regionDataZSize, &regionDataStart, &regionGPUBuffer);

	cl_mem densityGPUBuffer = NULL;
	if (useGPU) densityGPUBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(char) * REGION_SIZE * REGION_SIZE, NULL, NULL);
	generateDensityGrid(regionData, regionDataXSize, regionDataZSize, densityGrid, &densityGPUBuffer);
	int listLength = 0;
	//struct chunkPos* hotspotIndexList = generateHotspotIndexList(regionData, regionDataXSize, regionDataZSize, &listLength, threshold);
	int chunksThreshold = ((threshold - 1) >> 8) + 1;
	for (int zPos = 0; zPos < REGION_SIZE; zPos++) {
		for (int xPos = 0; xPos < REGION_SIZE; xPos++) {
			//printf("%d ", (int)(densityGrid[zPos][xPos]));

			if (densityGrid[zPos * REGION_SIZE + xPos] >= chunksThreshold) { // we've got a ceil function at home
				struct chunkPos hotspotPos = { xPos + regionStart.x, zPos + regionStart.z };
				struct blockPos hotspotMaxBlock = { 0, 0, 0 };

				int chunkCount = 0;
				int spawningSpaces = hotspotMaxSpawningSpaces(regionData, regionDataXSize, &regionStart, &hotspotPos, &hotspotMaxBlock, threshold, &chunkCount);

				if (spawningSpaces >= threshold) {
					printf("%d spawning spaces found from %d chunks at (%d, %d, %d)\n", spawningSpaces, chunkCount, hotspotMaxBlock.x, hotspotMaxBlock.y, hotspotMaxBlock.z);
				}
			}
		}
	}
	clReleaseMemObject(regionGPUBuffer);
	clReleaseMemObject(densityGPUBuffer);
}