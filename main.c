#include "mandellib.h"
#include <libswscale/swscale.h>
#define PARAMS 7


void genbar(double f,char** bar){
    char* tmp = *bar;
    tmp[0] = '\0';
    char* b = malloc(100*sizeof(char));
    char* escape = "\e[38;5;118m";
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
    const char* path = argv[4];
    const char* filename = argv[5];
    const double scalar_size = 1.0;
    const int height =(int) (720.0*scalar_size);
    const int width =(int) ((double)height*16./9.);
    const int speed = atoi(argv[6]);
    float zoom_factor = 0.5;
    const double realpart = -0.743643887037151;
    const double impart = 0.1318259045330;
    

    AVFormatContext* fmt_ctx = NULL;
    AVCodecContext* codec_ctx = NULL;
    AVStream* stream = NULL;
    AVCodec* codec = NULL;
    AVFrame* rgb_frame = NULL;
    AVFrame* yuv_frame = NULL;
    AVPacket* pkt = NULL;
    struct SwsContext* sws_ctx = NULL;

    avformat_network_init();
    avformat_alloc_output_context2(&fmt_ctx,NULL,"mp4",filename);
    if(!fmt_ctx){
        printf("Unable to crate output context.\n");
        exit(1);
    }
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec){
        printf("Unable to get Codec.\n");
        exit(1);
    }
    stream = avformat_new_stream(fmt_ctx,codec);
    if(!stream){
        printf("Unable to create stream.\n");
        exit(1);
    }
    stream->id = fmt_ctx->nb_streams-1;
    codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = av_make_q(1,fps);
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->gop_size = 30;
    stream->time_base = codec_ctx->time_base;
    stream->r_frame_rate =(AVRational) {fps,1};
    stream->avg_frame_rate = (AVRational) {fps,1};
    if(avcodec_parameters_from_context(stream->codecpar,codec_ctx)<0){
        printf("Failed to copy codec parameters to stream\n");
        exit(1);
    }
    stream->r_frame_rate =(AVRational) {fps,1};
    stream->avg_frame_rate = (AVRational) {fps,1};
    stream->time_base = (AVRational) av_make_q(fps,1);
    

    //Open encoder
    if(avcodec_open2(codec_ctx,codec,NULL)<0){
        printf("Unable to open encoder\n");
        exit(1);
    }
    rgb_frame = av_frame_alloc();
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = width;
    rgb_frame->height = height;
    av_frame_get_buffer(rgb_frame,0);//Allocate memory for pixels
    //Alocate YUV frame
    yuv_frame = av_frame_alloc();
    yuv_frame->format = AV_PIX_FMT_YUV420P;
    yuv_frame->width = width;
    yuv_frame->height =height;
    av_frame_get_buffer(yuv_frame,0);
    //Allocate memory for packet data
    pkt = av_packet_alloc();
    //Initialize RGB->YUV converter
    sws_ctx = sws_getContext(
        width,height,AV_PIX_FMT_RGB24,
        width,height,AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,NULL,NULL,NULL);
    
    //Open outputfile
    char* file_path = malloc(sizeof(char)*255);
    file_path = strcpy(file_path,path);
    strcat(file_path,filename);
    if(avio_open(&fmt_ctx->pb,file_path,AVIO_FLAG_WRITE)<0){
        printf("Unable to open output file\n");
        exit(1);
    }
    
    if(avformat_write_header(fmt_ctx,NULL)<0){
        printf("Unable to write header.\n");
        exit(1);
    }
    char* bar = malloc(256*sizeof(char));
    int nubmerOfFrames = fps * seconds;
    clock_t t_start = clock();

    //Encode frames
    Frame* computed_frame = initFrame(width,height,m_theme_get(RED_GRADIENT));
    for(int i = 0; i<nubmerOfFrames; i++){
        rgb_frame->pts =i;
        const double x_min = realpart-1.0/zoom_factor;
        const double x_max = realpart + 1.0/zoom_factor;
        const double y_min = (impart-1.0/zoom_factor) * 9.0/16.0;
        const double y_max = (impart + 1.0/zoom_factor)*9.0/16.0;
        createFrame(computed_frame,accuracy,x_min,x_max,y_min,y_max);
        zoom_factor *=1.0+ ((double)speed/(double)fps)/10.0;
        for(int y =0; y< height; y++){
            for(int x=0;x<width;x++){
                int offset = y*rgb_frame->linesize[0] + x*3;
                rgb_frame->data[0][offset] = (computed_frame->pixels[y][x]>>16) & 255;
                rgb_frame->data[0][offset+1] = (computed_frame->pixels[y][x]>>8) & 255;
                rgb_frame->data[0][offset+2] = (computed_frame->pixels[y][x])& 255;
            }
        }
        //Convert frame RGB to YUV 
        sws_scale(sws_ctx,rgb_frame->data,rgb_frame->linesize,
        0,height,yuv_frame->data,yuv_frame->linesize);
        yuv_frame->pts = i;
        //Encode 

        if(avcodec_send_frame(codec_ctx, yuv_frame) <0){
            break;
            //Error sending the frame
        }
        while(avcodec_receive_packet(codec_ctx,pkt)>=0){
            av_packet_rescale_ts(pkt,codec_ctx->time_base,stream->time_base);
            pkt->stream_index =stream->index;
            av_interleaved_write_frame(fmt_ctx,pkt);
            av_packet_unref(pkt);
        }
        genbar((double)(i+1)/(double)nubmerOfFrames,&bar);
        printf("\r\e[0m");
        printf("Frame: %d/%d ,%s",i+1,nubmerOfFrames,bar);
        fflush(stdout);
    }
    clock_t t_end = clock();
    printf("FINAL TIME: %ld\n",t_end - t_start);
    freeFrame(computed_frame);
    //Flush encoder and process last frames 
    avcodec_send_frame(codec_ctx,NULL);
    while(avcodec_receive_packet(codec_ctx,pkt)>=0){
        av_packet_rescale_ts(pkt,codec_ctx->time_base,stream->time_base);
        pkt->stream_index =stream->index;
        av_interleaved_write_frame(fmt_ctx,pkt);
        av_packet_unref(pkt);
    }

    av_write_trailer(fmt_ctx);
    sws_freeContext(sws_ctx);
    av_frame_free(&rgb_frame);
    av_frame_free(&yuv_frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avio_closep(&fmt_ctx->pb);
    avformat_free_context(fmt_ctx);
    free(file_path);
    return 0;
}