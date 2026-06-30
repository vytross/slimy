#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl.h>
#include "GPU.h"


cl_int findGPU() {
	cl_int err;
	cl_uint platformCount = 1;
	cl_platform_id platformID;
	extern cl_device_id gpu;
	extern cl_context context;

	err = clGetPlatformIDs(platformCount, &platformID, NULL);
	if (err != CL_SUCCESS) return err;

	err = clGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 1, &gpu, NULL);
	if (err != CL_SUCCESS) return err;

	context = clCreateContext(NULL, 1, &gpu, NULL, NULL, &err);
	return err;
}

cl_int shipDataToGPU(int* cpuDataArray, int length, cl_mem* buffer) {
	cl_int err;

	*buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * length, NULL, &err);
	if (err != CL_SUCCESS) return err;

	cl_command_queue queue;
	queue = clCreateCommandQueue(context, gpu, 0, &err);
	if (err != CL_SUCCESS) return err;

	err = clEnqueueWriteBuffer(queue, *buffer, CL_TRUE, 0, sizeof(int) * length, cpuDataArray, 0, NULL, NULL);
	return err;
}

cl_int readBufferFromGPU(int* output, int outputLength, cl_mem* buffer) {
	cl_int err;
	cl_command_queue queue;
	queue = clCreateCommandQueue(context, gpu, 0, &err);
	if (err != CL_SUCCESS) return err;

	err = clEnqueueReadBuffer(queue, *buffer, CL_TRUE, 0, sizeof(int) * outputLength, output, 0, NULL, NULL);
	return err;
}