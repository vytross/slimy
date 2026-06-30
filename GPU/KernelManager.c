#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>
#include "GPU.h"

extern cl_device_id gpu;
extern cl_context context;

cl_int buildKernels() {
	FILE* generateRegionFile = fopen("GPU/GenerateRegion.cl", "rb");
	FILE* generateXDensityGridFile = fopen("GPU/GenerateXDensityGrid.cl", "rb");
	FILE* generateDensityGridFile = fopen("GPU/GenerateDensityGrid.cl", "rb");
	//FILE* hotspotTestPointsFile = fopen("GPU/HotspotTestPoints.cl", "rb");

	extern cl_kernel generateRegionKernel;
	extern cl_kernel generateXDensityGridKernel;
	extern cl_kernel generateDensityGridKernel;
	//extern cl_kernel hotspotTestPointsKernel;

	extern cl_program generateRegionProgram;
	extern cl_program generateXDensityGridProgram;
	extern cl_program generateDensityGridProgram;
	//extern cl_program hotspotTestPointsProgram;

	// see GenerateRegion.cl for why i use this flag
	cl_int err = buildKernel(generateRegionFile, &generateRegionKernel, &generateRegionProgram, "generateRegion", "-cl-opt-disable");
	if (err != CL_SUCCESS) return err;

	err = buildKernel(generateXDensityGridFile, &generateXDensityGridKernel, &generateXDensityGridProgram, "generateXDensityGrid", NULL);
	if (err != CL_SUCCESS) return err;

	err = buildKernel(generateDensityGridFile, &generateDensityGridKernel, &generateDensityGridProgram, "generateDensityGrid", NULL);
	if (err != CL_SUCCESS) return err;

	//err = buildKernel(hotspotTestPointsFile, &hotspotTestPointsKernel, &hotspotTestPointsProgram, "hotspotTestPoints", NULL);
	//if (err != CL_SUCCESS) return err;

	fclose(generateRegionFile);
	fclose(generateXDensityGridFile);
	fclose(generateDensityGridFile);
	//fclose(hotspotTestPointsFile);

	return err;
}

cl_int buildKernel(FILE* kernelFile, cl_kernel* kernel, cl_program* program, char* name, char* options) {
	fseek(kernelFile, 0L, SEEK_END);
	long filesize = ftell(kernelFile);

	char* buffer = (char*)calloc(filesize + 1, sizeof(char));

	fseek(kernelFile, 0L, SEEK_SET);
	fread(buffer, sizeof(char), filesize, kernelFile);
	buffer[filesize] = '\0';
	
	cl_int err;

	program = clCreateProgramWithSource(context, 1, (const char* []) { buffer }, NULL, &err);
	if (err != CL_SUCCESS) return err;

	err = clBuildProgram(program, 1, &gpu, options, NULL, NULL); // that's right, we're rawdogging it
	if (err != CL_SUCCESS) { 
		size_t logsize;
		clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
		char* log = (char*)calloc(logsize, sizeof(char));
		err = clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, logsize, log, NULL);
		printf("%s", log);
		return err;
	}

	*kernel = clCreateKernel(program, name, &err);
	if (err != CL_SUCCESS) return err;

	free(buffer);
	return err;
}