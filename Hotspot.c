#include <stdlib.h>
#include "Slime.h"
#include "GPU/GPU.h"
#include "CircleTables.h"

#define TEST_POINT_MAX_ERROR 660

int hotspotMaxSpawningSpaces(int* regionData, int regionWidth, struct chunkPos* regionStart, struct chunkPos* hotspotPos, struct blockPos* maxSpacesPos, int threshold, int* chunks) {
	int xChunk = (*hotspotPos).x - (*regionStart).x;
	int zChunk = (*hotspotPos).z - (*regionStart).z;
	struct chunkPos hotspotIndex = { xChunk, zChunk };

	/* first check if there are even enough chunks to theoretically have more spawning spaces than the threshold. if not, don't
	 * even bother looking into this chunk anymore. we'll use this count anyway to make our slime chunk list the right size */
	int chunkCount = slimeChunkCount(regionData, regionWidth, &hotspotIndex, hotspotPos);
	if (chunkCount < ((threshold - 1) >> 8) + 1) return 0;

	*chunks = chunkCount;

	// generate a list of slime chunks
	int* chunkIndexList = (int*) calloc(chunkCount, sizeof(int));

	int chunkIndex = 0;
	int skipChunks[17] = { 4, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 4 };

	int xIndex, zIndex;
	for (zIndex = 0; zIndex < 17; zIndex++) {
		for (xIndex = skipChunks[zIndex]; xIndex < 17 - skipChunks[zIndex]; xIndex++) {
			if (regionData[((zChunk + zIndex) * regionWidth) + ((xChunk + xIndex) >> 5)] & (1 << ((xChunk + xIndex) % 32))) {
				chunkIndexList[chunkIndex] = (zIndex * 17) + xIndex;
				chunkIndex++;
			}
		}
	}
	
	int maxSpawningSpaces = hotspotSpawningSpacesCPUSearch(threshold, chunkIndexList, chunkCount, hotspotPos, maxSpacesPos);

	free(chunkIndexList);

	return maxSpawningSpaces;
}

int slimeChunkCount(int* regionData, int regionWidth, struct chunkPos* hotspotIndex, struct chunkPos* hotspotPos) {
	int circleBitmap[17] = { 0x1FF0, 0x7FFC, 0xFFFE, 0xFFFE, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0x1FFFF, 0xFFFE, 0xFFFE, 0x7FFC, 0x1FF0 };
	int chunkCount = 0;

	int xChunkStart = (*hotspotIndex).x;
	int zChunkStart = (*hotspotIndex).z;

	int zIndex, zChunk, xChunkLeftBits, xChunkRightBits, xChunkCount;
	for (zIndex = 0; zIndex < 17; zIndex++) {
		zChunk = zChunkStart + zIndex;

		xChunkLeftBits = regionData[(zChunk * regionWidth) + (xChunkStart >> 5)] & (circleBitmap[zIndex] << (xChunkStart % 32));
		xChunkRightBits = 0;
		if (xChunkStart % 32 >= 15) {
			xChunkRightBits = regionData[(zChunk * regionWidth) + (xChunkStart >> 5) + 1] & (circleBitmap[zIndex] >> (32 - (xChunkStart % 32)));
		}
		xChunkCount = xChunkLeftBits | xChunkRightBits;

		for (; xChunkCount; chunkCount++) {
			xChunkCount &= xChunkCount - 1;
		}
	}
	return chunkCount;
}

int hotspotSpawningSpacesCPUSearch(int threshold, int* chunkIndexList, int chunkListLength, struct chunkPos* hotspotPos, struct blockPos* maxPos) {
	//if (useGPU) return hotspotSpawningSpacesGPUSearch(threshold, chunkIndexList, chunkListLength, hotspotPos, maxPos);
	//else {
		int maxSpawningSpaces = 0;

		int xTest, yTest, zTest, spawningSpacesTest;
		int testIndex = 0;
		int xOffset, yOffset, zOffset, spawningSpaces;
		struct blockPos testPos, pos;
		for (yTest = 3; yTest < 22; yTest += 6) {
			for (zTest = 2; zTest < 15; zTest += 4) {
				for (xTest = 2; xTest < 15; xTest += 4) {
					testPos.x = xTest;
					testPos.y = yTest;
					testPos.z = zTest;

					spawningSpacesTest = countTestPointSpawningSpaces(chunkIndexList, chunkListLength, testIndex);

					int blockIndex = 0;

					if (spawningSpacesTest >= threshold - TEST_POINT_MAX_ERROR) {
						for (yOffset = -3; yOffset <= 3; yOffset++) {
							for (zOffset = -2; zOffset <= 1; zOffset++) {
								for (xOffset = -2; xOffset <= 1; xOffset++) {
									spawningSpaces = countSpawningSpacesAtPoint(chunkIndexList, chunkListLength, testIndex, blockIndex);

									if (spawningSpaces >= maxSpawningSpaces) {
										maxSpawningSpaces = spawningSpaces;
										maxPos->x = ((*hotspotPos).x * 16) + xTest + xOffset;
										maxPos->y = yTest + yOffset;
										maxPos->z = ((*hotspotPos).z * 16) + zTest + zOffset;
									}

									blockIndex++;
								}
							}
						}
					}
					testIndex++;
				}
			}
		}

		return maxSpawningSpaces;
	//}
}

int countTestPointSpawningSpaces(int* chunkIndexList, int chunkListLength, int testIndex) {
	extern int CPUTestPointTable[64 * 289];
	int spawningSpaces = 0;

	for (int i = 0; i < chunkListLength; i++) {
		spawningSpaces += CPUTestPointTable[(testIndex * 289) + chunkIndexList[i]];
	}

	return spawningSpaces;
}

int countSpawningSpacesAtPoint(int* chunkIndexList, int chunkListLength, int testIndex, int blockIndex) {
	extern int** CPUCubeletTables;
	//int* cubeletTable = CPUCubeletTables[testIndex];
	int spawningSpaces = 0;

	for (int i = 0; i < chunkListLength; i++) {
		spawningSpaces += CPUCubeletTables[testIndex][(blockIndex * 289) + chunkIndexList[i]];
	}
	return spawningSpaces;
}

/*
int hotspotSpawningSpacesGPUSearch(int threshold, int* chunkIndexList, int chunkListLength, struct chunkPos* hotspotPos, struct blockPos* maxPos) {
	int maxSpawningSpaces = 0;

	cl_int err;
	extern cl_kernel hotspotTestPointsKernel;
	extern cl_device_id gpu;
	extern cl_context context;
	extern cl_mem testPointTable;
	cl_command_queue queue = clCreateCommandQueue(context, gpu, 0, &err);

	//int testPointTableTest[64 * 289];
	//clEnqueueReadBuffer(queue, testPointTable, CL_TRUE, 0, sizeof(int) * 64 * 289, (void*)testPointTableTest, 0, NULL, NULL);

	cl_mem testPointsBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * 64, NULL, &err);
	cl_mem chunkIndexListBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * chunkListLength, NULL, &err);
	err = clEnqueueWriteBuffer(queue, chunkIndexListBuffer, CL_TRUE, 0, sizeof(int) * chunkListLength, chunkIndexList, 0, NULL, NULL);
	err = clSetKernelArg(hotspotTestPointsKernel, 0, sizeof(cl_mem), (void*)&testPointTable);
	err = clSetKernelArg(hotspotTestPointsKernel, 1, sizeof(cl_mem), (void*)&chunkIndexListBuffer);
	err = clSetKernelArg(hotspotTestPointsKernel, 2, sizeof(int), (void*)&chunkListLength);
	err = clSetKernelArg(hotspotTestPointsKernel, 3, sizeof(cl_mem), (void*)&testPointsBuffer);

	const size_t global = 64;
	cl_event event = NULL;
	err = clEnqueueNDRangeKernel(queue, hotspotTestPointsKernel, 1, NULL, &global, NULL, 0, NULL, &event);
	err = clWaitForEvents(1, &event);
	clReleaseEvent(event);

	int testPoints[64];
	clEnqueueReadBuffer(queue, testPointsBuffer, CL_TRUE, 0, sizeof(int) * 64, (void*)testPoints, 0, NULL, NULL);

	clReleaseMemObject(testPointsBuffer);

	int xOffset, yOffset, zOffset, spawningSpaces;
	int yTest = 3;
	int i = 0;
	struct blockPos pos;
	for (int y = 0; y < 4; y++) {
		int xTest = 2;
		for (int x = 0; x < 4; x++) {
			int zTest = 2;
			for (int z = 0; z < 4; z++) {
				if (testPoints[i] >= threshold - TEST_POINT_MAX_ERROR) {
					for (yOffset = -3; yOffset <= 3; yOffset++) {
						for (zOffset = -2; zOffset <= 1; zOffset++) {
							for (xOffset = -2; xOffset <= 1; xOffset++) {
								pos.x = xTest + xOffset;
								pos.y = yTest + yOffset;
								pos.z = zTest + zOffset;

								spawningSpaces = countSpawningSpacesAtPoint(chunkIndexList, chunkListLength, &pos);

								if (spawningSpaces >= maxSpawningSpaces) {
									maxSpawningSpaces = spawningSpaces;
									maxPos->x = ((*hotspotPos).x * 16) + xTest + xOffset;
									maxPos->y = yTest + yOffset;
									maxPos->z = ((*hotspotPos).z * 16) + zTest + zOffset;
								}
							}
						}
					}
				}
				i++;
				zTest += 4;
			}
			xTest += 4;
		}
		yTest += 6;
	}

	clReleaseMemObject(chunkIndexListBuffer);
	clReleaseCommandQueue(queue);
	return maxSpawningSpaces;
}
*/