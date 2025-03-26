#include "m_compute.h"
void printfCPU(char* cpu){
    printf("\033[38;5;115m%s\033[0m",cpu);
}
void printfGPU(char* gpu){
    printf("\033[38;5;118m%s\033[0m",gpu); 
}
void printfENCODER(char* encoder){
    printf("\033[38;5;126m%s\033[0m",encoder); 
}
M_Graphics initGraphics(){
    M_Graphics graphix;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_int err;
    printfGPU("Connecting GPU\n");
    //GET PLATFORM AND DEVICE 
    clGetPlatformIDs(1,&platform,NULL);
    clGetDeviceIDs(platform,CL_DEVICE_TYPE_GPU,1,&device,NULL);
    graphix.platform = platform;
    graphix.device = device;
    printfCPU("\033[38;5;115mCreating context and command\033[0m\n");
    //CREATE CONTEXT AND COMMAND
    context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
    queue = clCreateCommandQueueWithProperties(context, device, NULL , &err);
    graphix.queue = queue;
    graphix.context = context;
    if (err != CL_SUCCESS){
        printf("Error creating context and command\n");
        exit(1);
    }
    
    return graphix;
}
void initBufferGraphics(M_Graphics* gphx, M_DeviceParameters params,int width, int height){
    int numberOfFrames = params.mem_max_number_of_frames;
    printfCPU("Memory allocation on CPU\n");
    Virtual_Frame* frame_data = malloc(sizeof(Virtual_Frame)*numberOfFrames);
    int* escape_pixel_data = calloc(width*height*numberOfFrames,sizeof(int));
    if(!escape_pixel_data || !frame_data){
        printf("\033[38;5;196mError allocating the data on the CPU\033[0m\n");
        exit(1);
    }
    //printf("P1: %ld\n",sizeof(Virtual_Frame)*numberOfFrames);
    gphx->v_frames = frame_data;
    gphx->pixel_data = escape_pixel_data;
}
M_DeviceInfo getDeviceInfo(M_Graphics gphx){
    cl_ulong max_alloc_size;
    cl_device_id device = gphx.device;
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_alloc_size, NULL);
    cl_uint max_compute_units;
    cl_ulong max_work_group_size;
    size_t max_work_item_sizes[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_compute_units, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), &max_work_group_size, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, max_work_item_sizes, NULL);
    M_DeviceInfo info;
    info.max_alloc_size = max_alloc_size;
    info.max_compute_units = max_compute_units;
    info.max_work_group_size = max_work_group_size;
    info.max_work_item_sizes[0] = max_work_item_sizes[0];
    info.max_work_item_sizes[1] = max_work_item_sizes[1];
    info.max_work_item_sizes[2] = max_work_item_sizes[2];
    return info;
}
void printDeviceInfo(M_DeviceInfo dvInfo,M_DeviceParameters params){
    printf("Max buffer size: %lu bytes\n", dvInfo.max_alloc_size);
    printf("\033[38;5;118mMaximum parallel number of frame: %d\033[0m\n",params.mem_max_number_of_frames);
    printf("\033[38;5;118mNumber of batch: %d\033[0m\n",params.number_of_batch);
    printf("Allocated GPU threads to the program: %ld\n",params.max_threads);
    printf("GPU Redundancy: %ld\n",params.gpu_redundancy);
    printf("Compute Units (CUs): %u\n", dvInfo.max_compute_units);
    printf("Max Work-Group Size: %zu\n", dvInfo.max_work_group_size);
    printf("Max Work-Item Sizes: [%zu, %zu, %zu]\n", dvInfo.max_work_item_sizes[0], dvInfo.max_work_item_sizes[1], dvInfo.max_work_item_sizes[2]);
}
M_DeviceParameters calculateDeviceParameters(M_DeviceInfo f, int width, int height, int numberOfFrames){
    const int memory_limited_number_of_frame = (f.max_alloc_size)/(width*height*sizeof(int));
    
    const int number_of_batch = numberOfFrames/memory_limited_number_of_frame + !(numberOfFrames%memory_limited_number_of_frame==0);
    
    cl_ulong max_threads = f.max_compute_units * f.max_work_item_sizes[0] *7/8;//we allocate only 3/4 of max threads of the gpu
    cl_ulong redundancy = max_threads/memory_limited_number_of_frame;
    return (M_DeviceParameters) 
    {   .max_threads = max_threads,
        .gpu_redundancy=redundancy,
        .mem_max_number_of_frames=memory_limited_number_of_frame,
        .number_of_batch=number_of_batch,
        .threads_per_batch = redundancy * memory_limited_number_of_frame
    };
}
void createGPUBuffers(M_Graphics* gphx,M_DeviceParameters params,int width, int height){
    int batch_number_of_frame = params.mem_max_number_of_frames;
    printfGPU("Memory allocation on GPU\n");
    cl_mem frame_mem = clCreateBuffer(gphx->context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, batch_number_of_frame*sizeof(Virtual_Frame), gphx->v_frames, &(gphx->err));
    gphx->gpu_frames = frame_mem;
    cl_mem pixel_mem = clCreateBuffer(gphx->context,CL_MEM_READ_WRITE,batch_number_of_frame*width*height*sizeof(int), NULL,&(gphx->err));
    gphx->gpu_pixels = pixel_mem;
    cl_mem gpu_delta = clCreateBuffer(gphx->context,CL_MEM_READ_WRITE,sizeof(int),NULL,&(gphx->err));
    if (gphx->err != CL_SUCCESS){
        printf("Error creating buffers on GPU. Error: %d\n",gphx->err);
        exit(1);
    }
}
void createFrameData(M_Graphics gphx,M_Batch batch, int width, int height, int speed, int fps, double realpart, double impart){
    double zoom_factor;
    printf("\033[38;5;115mCreating frame data on CPU for range [%d-%d]\n\033[0m",batch.pos,batch.pos+batch.size);
    for(int i = batch.pos; i<batch.pos + batch.size; i++){
        zoom_factor = pow(1.0+ ((double)speed/(double)fps)/10.0,i);
        const double x_min = realpart-1.0/zoom_factor;
        const double x_max = realpart + 1.0/zoom_factor;
        const double y_min = impart-height/(zoom_factor*width);
        const double y_max = impart + height/(zoom_factor*width);
        Virtual_Frame v_frame = (Virtual_Frame){width,height,i,i-batch.pos,x_min,x_max,y_min,y_max};
        gphx.v_frames[i-batch.pos] = v_frame;
    }
}
void createProgram(M_Graphics* gphx,const char** sourcecode){
    //const char** s = sourcecode;
    gphx->program = clCreateProgramWithSource(gphx->context, 1, sourcecode, NULL, &(gphx->err));
    clBuildProgram(gphx->program,1,&(gphx->device),NULL,NULL,NULL);
}
void createKernel(M_Graphics* gphx,char* fct){
    printfCPU("Build kernel on CPU\n");
    gphx->kernel = clCreateKernel(gphx->program,fct,&(gphx->err));
}
void setKernelArguments(M_Graphics gphx,M_DeviceParameters params, int accuracy){
    printfCPU("Set kernel arguments\n");
    clSetKernelArg(gphx.kernel, 0, sizeof(cl_mem), &(gphx.gpu_frames));
    clSetKernelArg(gphx.kernel, 1, sizeof(cl_mem), &(gphx.gpu_pixels));
    clSetKernelArg(gphx.kernel, 2, sizeof(int), &(params.mem_max_number_of_frames));
    clSetKernelArg(gphx.kernel,3,sizeof(int),&accuracy);
    clSetKernelArg(gphx.kernel, 4, sizeof(int), &(params.gpu_redundancy));
}
void launchKernel(M_Graphics gphx,M_DeviceParameters params){
    printfGPU("Launch kernel\n");
    size_t gpu_threads = params.threads_per_batch;
    clEnqueueNDRangeKernel(gphx.queue,gphx.kernel,1,NULL,&gpu_threads,NULL,0,NULL,NULL);
    if (gphx.err != CL_SUCCESS){
        printf("Error sending the work-items\n");
        exit(1);
    }
}
void copyBufferToGPU(M_Graphics gphx,M_DeviceParameters params){
    gphx.err = clEnqueueWriteBuffer(gphx.queue,gphx.gpu_frames,CL_TRUE,0,sizeof(Virtual_Frame)*params.mem_max_number_of_frames,gphx.v_frames,0,NULL,NULL);
    if(gphx.err!=CL_SUCCESS){
        printfCPU("Error while trying to copy data to the GPU\n");
        exit(1);
    }
}
void freeGraphics(M_Graphics gphx){
    clReleaseMemObject(gphx.gpu_frames);
    clReleaseMemObject(gphx.gpu_pixels);
    clReleaseKernel(gphx.kernel);
    clReleaseCommandQueue(gphx.queue);
    clReleaseProgram(gphx.program);
    clReleaseContext(gphx.context);
    free(gphx.pixel_data);
    free(gphx.v_frames);
}
void computeFrame(M_Graphics gphx,M_DeviceParameters device_params,int height, int width, int numberOfFrames, int speed, int fps, int accuracy, double realpart, double impart, M_Theme_Prim* theme){
    int restant = numberOfFrames;
    for(int b = 0 ; b < device_params.number_of_batch;b++){
        int n_frame =(restant>device_params.mem_max_number_of_frames)?device_params.mem_max_number_of_frames:restant;
        M_Batch batch = {b*device_params.mem_max_number_of_frames,n_frame};
        createFrameData(gphx,batch,width,height,speed,fps,realpart,impart);
        //printf("TEST\n");
        copyBufferToGPU(gphx,device_params);
        //printf("TEST\n");
        launchKernel(gphx,device_params);
        printfGPU("Waiting for GPU results...\n");
        clFinish(gphx.queue);
        printfCPU("Copying frame data\n");
        clEnqueueReadBuffer(gphx.queue, gphx.gpu_pixels, CL_TRUE, 0, sizeof(int)*(width)*(height)*n_frame, gphx.pixel_data, 0, NULL, NULL);
        if (gphx.err != CL_SUCCESS){
            printf("Error reading data from GPU\n");
            exit(1);
        }
        clFinish(gphx.queue);
        printfENCODER("Beginning encoding frames\n");
        for(int i =0; i < n_frame;i++){
            Virtual_Frame frame = (gphx.v_frames)[i];
            printf("\rEncoding frame: %d\n",frame.frame_id);
            fflush(stdout);
            int* pixels = &(gphx.pixel_data)[width*height*frame.batch_id] ;
            encode(frame,pixels,accuracy,theme);
        }
        restant -= n_frame;
    }
}