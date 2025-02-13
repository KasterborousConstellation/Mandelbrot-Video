#include "mandellib.h"
#include "m_encoder.h"
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
    float zoom_factor = 0.5;
    const double realpart = -.743643887037151;
    const double impart = -.131825904205330;
    init_encoder(width,height,fps,filename,path);

    char* bar = malloc(256*sizeof(char));
    int nubmerOfFrames = fps * seconds;
    clock_t t_start = clock();
    
    //Encode frames
    Frame* computed_frame = initFrame(width,height,m_theme_get(COLORFUL_GRADIENT));
    for(int i = 0; i<nubmerOfFrames; i++){
        const double x_min = realpart-1.0/zoom_factor;
        const double x_max = realpart + 1.0/zoom_factor;
        const double y_min = impart-height/(zoom_factor*width);
        const double y_max = impart + height/(zoom_factor*width);
        createFrame(computed_frame,accuracy,x_min,x_max,y_min,y_max);
        zoom_factor *=1.0+ ((double)speed/(double)fps)/10.0;
        Virtual_Frame v_frame = (Virtual_Frame){width,height,i};
        encode(v_frame,computed_frame->pixels);
        genbar((double)(i+1)/(double)nubmerOfFrames,&bar);
        printf("\r\033[0m");
        printf("Frame: %d/%d ,%s",i+1,nubmerOfFrames,bar);
        fflush(stdout);
    }
    clock_t t_end = clock();
    printf("FINAL TIME: %ld\n",t_end - t_start);
    freeFrame(computed_frame);
    //Flush encoder and process last frames 
    flush_encoder();
    free_encoder();
    return 0;
}