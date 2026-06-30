#include <CL/cl.h>
#include <stdlib.h>
#include "Slime.h"
#include "GPU/GPU.h"
#include "CircleTables.h"

int** CPUCubeletTables = NULL;

cl_int generateCircleTables(_Bool useGPU) {
	// since the player can stand anywhere in a 16x25x16 region in a chunk, that's a LOT of math to check every single standing spot for
	// the number of spawning spaces surrounding the player. this thing sends several lookup tables to the GPU's memory to speed this up.

	 // we divide this area into 64 "cubelets" of size 4x6x4 (except for one 4x7x4) and order them by increasing z, then x, then y
	extern int CPUTestPointTable[64 * 289];
	CPUCubeletTables = calloc(sizeof(int*), 64);

	int pointIndex = 0;
	float yTestValues[4] = { 2.5, 8.5, 14.5, 21 };
	float xzTestValues[4] = { 1.5, 5.5, 9.5, 13.5 };
	extern cl_mem testPointTable;
	extern cl_mem cubeletTables[64];

	// i figure the compiler's probably smart enough to optimize this
	for (int y = 0; y < 4; y++) {
		for (int z = 0; z < 4; z++) {
			for (int x = 0; x < 4; x++) {
				int chunkIndex = 0;

				for (int zChunk = -8; zChunk <= 8; zChunk++) {
					for (int xChunk = -8; xChunk <= 8; xChunk++) {
						CPUTestPointTable[(pointIndex * 289) + chunkIndex] = 0;

						for (int zPos = 0; zPos < 16; zPos++) {
							for (int xPos = 0; xPos < 16; xPos++) {
								float xSquared = (xChunk * 16 + xPos - xzTestValues[x]) * (xChunk * 16 + xPos - xzTestValues[x]);
								float ySquared = yTestValues[y] * yTestValues[y];
								float zSquared = (zChunk * 16 + zPos - xzTestValues[z]) * (zChunk * 16 + zPos - xzTestValues[z]);
								float distanceSquared = xSquared + ySquared + zSquared;

								if (distanceSquared <= 16384.0F && distanceSquared > 576.0F) {
									CPUTestPointTable[(pointIndex * 289) + chunkIndex]++;
								}
							}
						}

						chunkIndex++;
					}
				}

				pointIndex++;
			}
		}
	}

	if (useGPU) {
		cl_int err = shipDataToGPU(CPUTestPointTable, 64 * 289, &testPointTable);
		if (err != CL_SUCCESS) return err;
	}

	for (int cubeletCount = 0; cubeletCount < 64; cubeletCount++) {
		// this itself might be best sent to the GPU. it takes a couple seconds each run

		int xStart = (cubeletCount % 4) * 4;
		int zStart = ((cubeletCount >> 2) % 4) * 4;
		int yStart = (cubeletCount >> 4) * 6;

		int yRange = 7;

		CPUCubeletTables[cubeletCount] = calloc(16 * yRange * 289, sizeof(int));

		int pointIndex = 0;
		for (int y = yStart; y < yStart + yRange; y++) {
			for (int z = zStart; z < zStart + 4; z++) {
				for (int x = xStart; x < xStart + 4; x++) {
					int chunkIndex = 0;

					for (int zChunk = -8; zChunk <= 8; zChunk++) {
						for (int xChunk = -8; xChunk <= 8; xChunk++) {
							CPUCubeletTables[cubeletCount][(pointIndex * 289) + chunkIndex] = 0;

							for (int zPos = 0; zPos < 16; zPos++) {
								for (int xPos = 0; xPos < 16; xPos++) {
									int xSquared = (xChunk * 16 + xPos - x) * (xChunk * 16 + xPos - x);
									int ySquared = y * y;
									int zSquared = (zChunk * 16 + zPos - z) * (zChunk * 16 + zPos - z);
									int distanceSquared = xSquared + ySquared + zSquared;

									if (distanceSquared <= 16384 && distanceSquared > 576) {
										CPUCubeletTables[cubeletCount][(pointIndex * 289) + chunkIndex]++;
									}
								}
							}

							chunkIndex++;
						}
					}

					pointIndex++;
				}
			}
		}

		if (useGPU) {
			cl_int err = shipDataToGPU(CPUCubeletTables[cubeletCount], 16 * yRange * 289, &cubeletTables[cubeletCount]);
			if (err != CL_SUCCESS) return err;
		}
	}

	return 0;
}