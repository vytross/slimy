#include <CL/cl.h>
#include <stdio.h>

cl_device_id gpu;
cl_context context;
cl_mem testPointTable;
cl_mem cubeletTables[64];

cl_int findGPU();
cl_int shipDataToGPU(int*, int, cl_mem*);
cl_int readBufferFromGPU(int*, int, cl_mem*);

cl_int generateGPUCircleTables();

cl_int buildKernels();
cl_int buildKernel(FILE*, cl_kernel*, cl_program*, char*, char*);

cl_kernel generateRegionKernel;
cl_program generateRegionProgram;

cl_kernel generateXDensityGridKernel, generateDensityGridKernel;
cl_program generateXDensityGridProgram, generateDensityGridProgram;

//cl_kernel hotspotTestPointsKernel;
//cl_program hotspotTestPointsProgram;