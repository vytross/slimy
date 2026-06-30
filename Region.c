// no relation to minecraft regions

#include <stdlib.h>
#include <CL/cl.h>
#include "Slime.h"
#include "GPU/GPU.h"

extern cl_device_id gpu;
extern cl_context context;
extern _Bool useGPU;

void generateRegion(int* region, int xSize, int zSize, struct chunkPos* regionStart, cl_mem* regionBuffer) {
	if (*regionBuffer) generateRegionGPU(region, xSize, zSize, regionStart, regionBuffer);
	else {
		extern uint64_t worldseed;

		uint64_t zseed;
		uint64_t seed;
		int xChunk, zChunk;
		int x, z, i;

		for (z = 0; z < zSize; z++) {
			zChunk = (*regionStart).z + z;
			zseed = worldseed 
				+ (uint64_t)(int32_t)(zChunk * zChunk) * 0x4307A7ULL
				+ (uint64_t)(int32_t)(zChunk * 0x5F24F);

			for (x = 0; x < xSize; x++) {
				region[z * xSize + x] = 0;
				xChunk = (*regionStart).x + (x << 5);
				for (i = 0; i < 32; i++) {
					seed = zseed
						+ (uint64_t)(int32_t)(xChunk * xChunk * 0x4C1906)
						+ (uint64_t)(int32_t)(xChunk * 0x5AC0DB);

					seed = ((seed ^ 0x5E434E432ULL) * 0x5DEECE66DULL + 0xB) & 0xFFFFFFFFFFFFULL;
					region[z * xSize + x] += (!((seed >> 17) % 10)) << i;

					xChunk++;
				}
			}
		}
	}
}

cl_int generateRegionGPU(int* regionData, int xSize, int zSize, struct chunkPos* regionStart, cl_mem* regionBuffer) {
	cl_int err;
	extern cl_kernel generateRegionKernel;
	extern cl_device_id gpu;
	extern cl_context context;
	cl_command_queue queue = clCreateCommandQueue(context, gpu, 0, &err);

	extern uint64_t worldseed;
	int width = xSize;
	int xRegionStart = (*regionStart).x;
	int zRegionStart = (*regionStart).z;

	err = clSetKernelArg(generateRegionKernel, 0, sizeof(int), (void*)&width);
	err = clSetKernelArg(generateRegionKernel, 1, sizeof(uint64_t), (void*)&worldseed);
	err = clSetKernelArg(generateRegionKernel, 2, sizeof(int), (void*)&xRegionStart);
	err = clSetKernelArg(generateRegionKernel, 3, sizeof(int), (void*)&zRegionStart);
	err = clSetKernelArg(generateRegionKernel, 4, sizeof(cl_mem), (void*)regionBuffer);

	const size_t global[2] = { xSize, zSize };
	cl_event event = NULL;
	err = clEnqueueNDRangeKernel(queue, generateRegionKernel, 2, NULL, global, NULL, 0, NULL, &event);
	err = clWaitForEvents(1, &event);
	clReleaseEvent(event);

	if (err != CL_SUCCESS) return err;

	clEnqueueReadBuffer(queue, *regionBuffer, CL_TRUE, 0, sizeof(int) * xSize * zSize, regionData, 0, NULL, NULL);
	clReleaseCommandQueue(queue);
	return err;
}

// leetcode ass function. also maybe worth sending to gpu. would be a good bit harder but like, look at all these for loops
void generateDensityGrid(int* region, int xSize, int zSize, char* density, cl_mem* densityGridBuffer) {
	if (*densityGridBuffer) generateDensityGridGPU(region, xSize, zSize, density, densityGridBuffer);
	else {
		char** xAxisDensity = malloc(zSize * sizeof(char*));

		int x, z, i;
		int xIndex;
		int chunkCount;

		uint64_t localRegionData;

		for (x = 0; x < (xSize << 5) - 16; x++) density[x] = 0;

		for (z = 0; z < zSize; z++) {
			xAxisDensity[z] = calloc(((xSize << 5) - 16), sizeof(char));
			xIndex = 0;

			for (x = 0; x < xSize - 1; x++) {
				localRegionData = (uint64_t)(uint32_t)region[(z * xSize) + x] + (((uint64_t)(uint32_t)region[(z * xSize) + x + 1]) << 32);
				for (i = 0; i < 32; i++) {
					chunkCount = (int)((localRegionData >> i) & 0x1FFFF);
					while (chunkCount) {
						xAxisDensity[z][xIndex]++;
						chunkCount &= chunkCount - 1;
					}

					if (z < 17) density[xIndex] += xAxisDensity[z][xIndex];
					else density[(z - 16) * (zSize - 16) + xIndex] = density[(z - 17) * (zSize - 16) + xIndex] - xAxisDensity[z - 17][xIndex] + xAxisDensity[z][xIndex];

					xIndex++;
				}
			}

			for (i = 32; i < 48; i++) {
				chunkCount = (int)((localRegionData >> i) & 0x1FFFF);
				while (chunkCount) {
					xAxisDensity[z][xIndex]++;
					chunkCount &= chunkCount - 1;
				}

				if (z < 17) density[xIndex] += xAxisDensity[z][xIndex];
				else density[(z - 16) * (zSize - 16) + xIndex] = density[(z - 17) * (zSize - 17) + xIndex] - xAxisDensity[z - 17][xIndex] + xAxisDensity[z][xIndex];

				xIndex++;
			}
			if (z >= 17) free(xAxisDensity[z - 17]);
		}

		for (z = zSize - 17; z < zSize; z++) free(xAxisDensity[z]);
		free(xAxisDensity);
	}
}

cl_int generateDensityGridGPU(int* region, int xSize, int zSize, char* densityGrid, cl_mem* densityGridBuffer) {
	cl_int err;
	extern cl_kernel generateDensityGridKernel, generateXDensityGridKernel;
	extern cl_device_id gpu;
	extern cl_context context;
	cl_command_queue queue = clCreateCommandQueue(context, gpu, 0, &err);

	cl_mem regionDataBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * xSize * zSize, NULL, &err);
	err = clEnqueueWriteBuffer(queue, regionDataBuffer, CL_TRUE, 0, sizeof(int) * xSize * zSize, region, 0, NULL, NULL);
	cl_mem xDensityBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(char) * zSize * (zSize - 16), NULL, &err);
	err = clSetKernelArg(generateXDensityGridKernel, 0, sizeof(cl_mem), (void*)&regionDataBuffer);
	err = clSetKernelArg(generateXDensityGridKernel, 1, sizeof(int), (void*)&xSize);
	err = clSetKernelArg(generateXDensityGridKernel, 2, sizeof(int), (void*)&zSize);
	err = clSetKernelArg(generateXDensityGridKernel, 3, sizeof(cl_mem), (void*)&xDensityBuffer);

	int densityGridSize = zSize - 16;
	const size_t globalX[2] = { zSize - 16, zSize };
	const size_t local[2] = {16, 16};

	cl_event eventX = NULL;
	err = clEnqueueNDRangeKernel(queue, generateXDensityGridKernel, 2, NULL, globalX, local, 0, NULL, &eventX);
	err = clWaitForEvents(1, &eventX);
	clReleaseEvent(eventX);

	//int* xDensity = malloc(zSize * (zSize - 16) * sizeof(int));
	//err = clEnqueueReadBuffer(queue, xDensityBuffer, CL_TRUE, 0, (zSize * (zSize - 16) * sizeof(int)), xDensity, 0, NULL, NULL);
	/*
	for (int z = 0; z < zSize; z++) {
		for (int x = 0; x < zSize - 16; x++) {
			printf("%d", xDensity[x * zSize + z]);
			//printf("%d", region[z * xSize + (x >> 5)]);
		}
	}*/

	if (err != CL_SUCCESS) return err;

	err = clSetKernelArg(generateDensityGridKernel, 0, sizeof(cl_mem), (void*)&xDensityBuffer);
	err = clSetKernelArg(generateDensityGridKernel, 1, sizeof(int), (void*)&densityGridSize);
	err = clSetKernelArg(generateDensityGridKernel, 2, sizeof(cl_mem), (void*)densityGridBuffer);

	const size_t global[2] = { zSize - 16, zSize - 16 };

	cl_event event = NULL;
	err = clEnqueueNDRangeKernel(queue, generateDensityGridKernel, 2, NULL, global, local, 0, NULL, &event);
	err = clWaitForEvents(1, &event);
	clReleaseEvent(event);

	if (err != CL_SUCCESS) return err;

	//char* densityGrid = malloc(densityGridSize * densityGridSize * sizeof(char));
	clEnqueueReadBuffer(queue, *densityGridBuffer, CL_TRUE, 0, densityGridSize * densityGridSize * sizeof(char), densityGrid, 0, NULL, NULL);

	clReleaseMemObject(regionDataBuffer);
	clReleaseMemObject(xDensityBuffer);

	clReleaseCommandQueue(queue);
	/*
	for (int z = 0; z < densityGridSize; z++) {
		for (int x = 0; x < densityGridSize; x++) {
			printf("%d ", (int)(densityGrid[z * densityGridSize + x]));
		}
	}*/

	return err;
}
/*
void generateDensityGrid(int* region, int xSize, int zSize, char* density) {
	char** xAxisDensity = malloc(zSize * sizeof(char*));

	int x, z;
	int densityXSize = (xSize << 5) - 16;
	int densityZSize = zSize - 16;
	int chunkCount;
	for (x = 0; x < densityXSize; x++) density[x * densityZSize] = 0;

	for (z = 0; z < zSize; z++) {
		xAxisDensity[z] = malloc(densityXSize * sizeof(char));

		for (x = 0; x < densityXSize; x++) {
			xAxisDensity[z][x] = 0;
			chunkCount = (region[(z * xSize) + (x >> 5)] & (0x1FFFF << (x % 32))) | ((x % 32 < 15) ? 0 : (region[(z * xSize) + (x >> 5) + 1] & (0x1FFFF >> (32 - (x % 32)))));
			for (; chunkCount; (xAxisDensity[z][x])++) chunkCount &= (chunkCount - 1);

			if (z < 17) density[x * densityZSize] += xAxisDensity[z][x];
			else density[(x * densityZSize) + z - 16] = density[(x * densityZSize) + z - 17] - xAxisDensity[z - 17][x] + xAxisDensity[z][x];
		}
		//if (z >= 18) free(xAxisDensity[z - 18]);
	}

	for (z = 0; z < zSize; z++) free(xAxisDensity[z]);
	free(xAxisDensity);
}*/

struct chunkPos* generateHotspotIndexList(int* region, int xSize, int zSize, int* listLength, int threshold) {
	int densityXSize = (xSize << 5) - 16;
	int chunksThreshold = ((threshold - 1) >> 8) + 1;
	struct chunkPos* indexList = calloc(densityXSize * (zSize - 16), sizeof(struct chunkPos));
	int indexListSize = densityXSize * (zSize - 16);
	struct chunkPos currentChunk;
	int indexListCount = 0;
	int nonHotspotCount = 0;

	int* xAxisDensity = calloc(18 * densityXSize, sizeof(int));
	int* xAxisDensityVal;
	int* density = calloc(18 * densityXSize, sizeof(int));

	int x, z, i;
	for (x = 0; x < densityXSize; x++) density[x * 18] = 0;

	uint64_t localRegionData;
	uint64_t chunkCount;
	for (z = 0; z < zSize; z++) {
		x = 0;
		localRegionData = region[z * xSize];
		for (i = 0; i < 16; i++) {
			chunkCount = localRegionData & (0x1FFFFULL << i);
			for (; chunkCount; (xAxisDensity[(z % 18) * densityXSize + i])++) chunkCount &= chunkCount - 1;

			if (z < 17) density[i * 18] += xAxisDensity[i];
			else {
				density[i * 18 + ((z - 16) % 18)] = density[i * 18 + ((z - 17) % 18)] - xAxisDensity[((z - 17) % 18) * densityXSize + i] + xAxisDensity[(z % 18) * densityXSize + i];
				//density[((z - 16) % 18) * densityXSize + i] = density[((z - 17) % 18) * densityXSize + i] - xAxisDensity[((z - 17) % 18) * densityXSize + i] + xAxisDensity[(z % 18) * densityXSize + i];
				if (density[i * 18 + ((z - 16) % 18)] >= chunksThreshold) {
				//if (density[((z - 16) * densityXSize) % 18 + x] >= chunksThreshold) {
					currentChunk.x = i + 16;
					currentChunk.z = z;
					indexList[indexListCount] = currentChunk;
					indexListCount++;
				}
				else {
					nonHotspotCount++;
				}
			}
		}
		for (x = 0; x < xSize - 1; x++) {
			localRegionData = region[z * xSize + x];
			for (i = 16; i < 48; i++) {
				chunkCount = localRegionData & (0x1FFFFULL << i);
				for (; chunkCount; (xAxisDensity[(z % 18) * densityXSize + i])++) chunkCount &= chunkCount - 1;

				if (z < 17) density[((x << 5) + i) * 18] += xAxisDensity[(x << 5) + i];
				else {
					density[((x << 5) + i) * 18 + ((z - 16) % 18)] = density[((x << 5) + i) * 18 + ((z - 17) % 18)] - xAxisDensity[((z - 17) % 18) * densityXSize + (x << 5) + i] + xAxisDensity[(z % 18) * densityXSize + (x << 5) + i];
					//density[((z - 16) % 18) * densityXSize + (x << 5) + i] = density[((z - 17) % 18) * densityXSize + (x << 5) + i] - xAxisDensity[((z - 17) % 18) * densityXSize + (x << 5) + i] + xAxisDensity[(z % 18) * densityXSize + (x << 5) + i];
					if (density[((x << 5) + i) * 18 + ((z - 16) % 18)] >= chunksThreshold) {
					//if (density[((z - 16) * densityXSize) % 18 + x] >= chunksThreshold) {
						currentChunk.x = (x << 5) + i;
						currentChunk.z = z;
						indexList[indexListCount] = currentChunk;
						indexListCount++;
					}
					else {
						nonHotspotCount++;
					}
				}
			}
		}
	}

	free(xAxisDensity);
	free(density);

	*listLength = indexListCount;

	struct chunkPos* resizedIndexList = realloc(indexList, indexListCount * sizeof(struct chunkPos));
	return resizedIndexList;
}