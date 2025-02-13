#include "mandellib.h"
static inline int FLAT(int x, int y, int linesize){
    return y*linesize +x;
}
static inline int COAL(int* pixels, int x, int y, int linesize){
    return pixels[FLAT(x,y,linesize)];
}

Frame* initFrame(const int width, const int height, M_Theme_Prim* theme){
    Frame* frame =(Frame*) malloc(sizeof(Frame));
    frame->height = height;
    frame->width =width;
    frame->theme = theme;
    frame->pixels =(int*) malloc(height*width*sizeof(int));
    return frame;
}
static inline int computeAt(int maxIter, double crp, double cip){
    int n;
    double rp = 0;
    double ip = 0;
    for(n=0;n<maxIter;n++){
        double srp = rp * rp;
        double sri = ip * ip;
        if(srp + sri > 4.0){
            break;
        }
        //Computes z² + c
        double new_rp = srp - sri + crp;
        double new_ip = 2*rp*ip +cip;
        rp = new_rp;
        ip = new_ip;
    }
    return n;
}

void createFrame(Frame* frame, const int accuracy, const double x_min, const double x_max, const double y_min, const double y_max){
    int width = frame->width;
    int height = frame->height;
    for(int y =0 ; y<height; y++){
        for(int x=0; x<width;x++){
            double creal = x_min + (double)x/(double)width *(x_max-x_min);
            double cim = y_max - ((double)y/(double)height) *(y_max-y_min); 
            int iter = computeAt(accuracy,creal,cim);
            int col= frame->theme->fptr(iter,accuracy);
            frame->pixels[FLAT(x,y,width)] = col;
        }
    }
}
void writeImage(const char* path, Frame* frame){
    FILE* fp = fopen(path,"wb");
    //Considering that fp is a .ppm
    if(!fp){
        perror("The file can't be openned.");
        exit(1);
    }
    fprintf(fp,"P6\n%d %d\n255\n",frame->width,frame->height);
    for(int y =0; y < frame->height;y++){
        
        for(int x=0; x < frame->width;x++){
            int pixel = frame->pixels[FLAT(x,y,frame->width)];
            fputc((pixel>>16) &255,fp);
            fputc((pixel>>8) &255,fp);
            fputc((pixel&255),fp);
        }
    }
    fclose(fp);
    
}
void directWriteImage(const char* path, int width, int height, int accuracy, double x_min,double x_max, double y_min, double y_max){
    FILE* fp = fopen(path,"wb");
    //Considering that fp is a .ppm
    if(!fp){
        perror("The file can't be openned.");
        exit(1);
    }
    long total_iter = height*width;
    long iter_loop = 0;
    fprintf(fp,"P6\n%d %d\n255\n",width,height);
    clock_t t_last = clock();
    for(int y =0; y < height;y++){
        for(int x=0; x < width;x++){
            clock_t now = clock();
            if(now-t_last>=CLOCKS_PER_SEC*10){
                printf("Work: %lf\n", 100.0*((double)iter_loop/(double)total_iter));
                t_last =clock();
            }
            double creal = x_min + (double)x/(width) *(x_max-x_min);
            double cim = y_max - (double)y/(height)*(y_max-y_min); 
            int iter = computeAt(accuracy,creal,cim);
            int pixel= 0;
            if(iter<accuracy){
                double t = (double)iter/(double)accuracy;
                pixel |= 255<<16;//Channel red
                pixel |= (int)(255.0- 255.0 * t)<<8;//Channel green
                pixel |=(int)(255.0- 255.0 * t);//Channel blue
            }
            fputc((pixel>>16) &255,fp);
            fputc((pixel>>8) &255,fp);
            fputc((pixel&255),fp);
            iter_loop++;
        }
    }
    fclose(fp);
    printf("Image processing completed\n");
}
void freeFrame(Frame* frame){
    free(frame->theme);
    free(frame->pixels);
    free(frame);
}