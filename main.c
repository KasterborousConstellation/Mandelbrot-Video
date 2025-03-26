#include "m_encoder.h"
#include "fileUtils.h"
#include "m_compute.h"
#define PARAMS 8
void genbar(double f,char** bar){
    char* tmp = *bar;
    tmp[0] = '\0';
    char* b = malloc(100*sizeof(char));
    char* escape = "\033[38;5;118m";
    int n_ash =(int) (f * 25.0);
    int delta =1;
    b[0] = '[';
    for(int i =0; i < 25;i++){
        if(i < n_ash){
            b[delta + i] = '#';
        }else{
            b[delta + i] = ' ';
        }
    }
    b[25 + delta] = ']';
    b[26 + delta] = '\0';
    strcat(tmp,escape);
    strcat(tmp,b);
    int N = strlen(tmp);
    for(int i =0; i <N;i++){
        (*bar)[i] = tmp[i];
    }
    free(b);
}
int main(int argc, char** argv){
    if(argc != PARAMS){
        printf("Wrong number of parameters\n");
        exit(1);
    }
    const int seconds = atoi(argv[1]);
    const int fps = atoi(argv[2]);
    const int accuracy = atoi(argv[3]);
    char* path = argv[4];
    char* filename = argv[5];
    const double scalar_size =(double) atoi(argv[7]);
    const int height =(int) (720.0*scalar_size);
    const int width =(int) ((double)height*16.0/9.);
    const int speed = atoi(argv[6]);
    const double realpart = -.743643887037151;
    const double impart = -.131825904205330;
    const int numberOfFrames = fps * seconds;
    //Init encoder
    init_encoder(width,height,fps,filename,path);
    //Init graphics
    M_Graphics gphx = initGraphics();
    //Read openCL code
    const char* openClKernelSource = read_entire_file("kernel.cl",NULL);
    char* bar = malloc(256*sizeof(char));
    //Calculate Device params
    
    M_DeviceInfo deviceinfo = getDeviceInfo(gphx);
    M_DeviceParameters device_params =  calculateDeviceParameters(deviceinfo,width,height,numberOfFrames);
    //Init memory on the CPU SIDE
    initBufferGraphics(&gphx,device_params,width,height);
    //PRINT THREADING INFO
    printDeviceInfo(deviceinfo,device_params);
    //Init memory on the GPU SIDE
    createGPUBuffers(&gphx,device_params,width,height);
    //Build the kernel
    createProgram(&gphx,&openClKernelSource);
    createKernel(&gphx,"calculate_frame");
    //Set kernel arguments
    setKernelArguments(gphx,device_params,accuracy);
    //Encode frames
    M_Theme_Prim* theme = m_theme_get(COLORFUL_GRADIENT);
    computeFrame(gphx,device_params,height,width,numberOfFrames,speed,fps,accuracy,realpart,impart,theme);
    free(theme);
    //Flush encoder and process last frames
    flush_encoder();
    free_encoder();
    //Release GPU memory
    freeGraphics(gphx);
    free(bar);
    free(openClKernelSource);
    return 0;
}