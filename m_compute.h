#ifndef M_COMPUTE
#define M_COMPUTE
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include "m_encoder.h"
typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_int err;
    Virtual_Frame* v_frames;
    int* pixel_data;
    cl_mem gpu_pixels;
    cl_mem gpu_frames;
}M_Graphics;
typedef struct{
    cl_ulong max_alloc_size;
    cl_uint max_compute_units;
    cl_ulong max_work_group_size;
    size_t max_work_item_sizes[3];
} M_DeviceInfo;
typedef struct{
    cl_ulong max_threads;
    cl_ulong gpu_redundancy;
    int mem_max_number_of_frames;
    int number_of_batch;
    int threads_per_batch;
} M_DeviceParameters;
typedef struct {
    int pos;
    int size;
} M_Batch;
M_Graphics initGraphics();
void initBufferGraphics(M_Graphics* gphx, M_DeviceParameters params,int width, int height);
M_DeviceInfo getDeviceInfo(M_Graphics gphx);
M_DeviceParameters calculateDeviceParameters(M_DeviceInfo f, int width, int height, int numberOfFrames);
void createGPUBuffers(M_Graphics *gphx,M_DeviceParameters params,int width, int height);
void printfCPU(char* cpu);
void printfGPU(char* gpu);
void printfENCODER(char* encoder);
void printDeviceInfo(M_DeviceInfo dvInfo,M_DeviceParameters params);
void createFrameData(M_Graphics gphx,M_Batch batch, int width, int height, int speed, int fps, double realpart, double impart);
void createProgram(M_Graphics* gphx, const char** sourcecode);
void createKernel(M_Graphics* gphx, char* kernelfunction);
void setKernelArguments(M_Graphics gphx,M_DeviceParameters params, int accuracy);
void launchKernel(M_Graphics gphx,M_DeviceParameters params);
void copyBufferToGPU(M_Graphics gphx,M_DeviceParameters params);
void freeGraphics(M_Graphics gphx);
void computeFrame(M_Graphics gphx,M_DeviceParameters device_params,int height, int width, int numberOfFrames, int speed, int fps, int accuracy, double realpart, double impart, M_Theme_Prim* theme);
#endif