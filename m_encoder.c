#include "m_encoder.h"
static inline int FLAT(int x, int y, int linesize){
    return y*linesize +x;
}
static inline int COAL(int* pixels, int x, int y, int linesize){
    return pixels[FLAT(x,y,linesize)];
}
struct M_Encoder KTBS_encoder;
void init_encoder(int width, int height, int fps,char* filename, char* path){
    avformat_network_init();
    avformat_alloc_output_context2(&KTBS_encoder.fmt_ctx,NULL,"mp4",filename);
    if(!KTBS_encoder.fmt_ctx){
        printf("Unable to crate output context.\n");
        exit(1);
    }
    KTBS_encoder.codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!KTBS_encoder.codec){
        printf("Unable to get Codec.\n");
        exit(1);
    }
    KTBS_encoder.stream = avformat_new_stream(KTBS_encoder.fmt_ctx,KTBS_encoder.codec);
    if(!KTBS_encoder.stream){
        printf("Unable to create stream.\n");
        exit(1);
    }
    KTBS_encoder.stream->id = KTBS_encoder.fmt_ctx->nb_streams-1;
    KTBS_encoder.codec_ctx = avcodec_alloc_context3(KTBS_encoder.codec);
    KTBS_encoder.codec_ctx->width = width;
    KTBS_encoder.codec_ctx->height = height;
    KTBS_encoder.codec_ctx->time_base = av_make_q(1,fps);
    KTBS_encoder.codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    KTBS_encoder.codec_ctx->gop_size = 30;
    KTBS_encoder.stream->time_base = KTBS_encoder.codec_ctx->time_base;
    KTBS_encoder.stream->r_frame_rate =(AVRational) {fps,1};
    KTBS_encoder.stream->avg_frame_rate = (AVRational) {fps,1};
    if(avcodec_parameters_from_context(KTBS_encoder.stream->codecpar,KTBS_encoder.codec_ctx)<0){
        printf("Failed to copy codec parameters to stream\n");
        exit(1);
    }
    KTBS_encoder.stream->r_frame_rate =(AVRational) {fps,1};
    KTBS_encoder.stream->avg_frame_rate = (AVRational) {fps,1};
    KTBS_encoder.stream->time_base = (AVRational) av_make_q(fps,1);
    

    //Open encoder
    if(avcodec_open2(KTBS_encoder.codec_ctx,KTBS_encoder.codec,NULL)<0){
        printf("Unable to open encoder\n");
        exit(1);
    }
    KTBS_encoder.rgb_frame = av_frame_alloc();
    KTBS_encoder.rgb_frame->format = AV_PIX_FMT_RGB24;
    KTBS_encoder.rgb_frame->width = width;
    KTBS_encoder.rgb_frame->height = height;
    av_frame_get_buffer(KTBS_encoder.rgb_frame,0);//Allocate memory for pixels
    //Alocate YUV frame
    KTBS_encoder.yuv_frame = av_frame_alloc();
    KTBS_encoder.yuv_frame->format = AV_PIX_FMT_YUV420P;
    KTBS_encoder.yuv_frame->width = width;
    KTBS_encoder.yuv_frame->height =height;
    av_frame_get_buffer(KTBS_encoder.yuv_frame,0);
    //Allocate memory for packet data
    KTBS_encoder.pkt = av_packet_alloc();
    //Initialize RGB->YUV converter
    KTBS_encoder.sws_ctx = sws_getContext(
        width,height,AV_PIX_FMT_RGB24,
        width,height,AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,NULL,NULL,NULL);
    
    //Open outputfile
    char* file_path = malloc(sizeof(char)*255);
    file_path = strcpy(file_path,path);
    strcat(file_path,filename);
    if(avio_open(&KTBS_encoder.fmt_ctx->pb,file_path,AVIO_FLAG_WRITE)<0){
        printf("Unable to open output file\n");
        exit(1);
    }
    
    if(avformat_write_header(KTBS_encoder.fmt_ctx,NULL)<0){
        printf("Unable to write header.\n");
        exit(1);
    }
    //free(file_path);
}

void encode(Virtual_Frame v_frame, int* pixels, int accuracy, M_Theme_Prim* theme){
    KTBS_encoder.rgb_frame->pts =v_frame.frame_id;
    for(int y =0; y< v_frame.height; y++){
        for(int x=0;x<v_frame.width;x++){
            int offset = y*(KTBS_encoder.rgb_frame->linesize[0]) + x*3;
            int data = COAL(pixels,x,y,v_frame.width);
            int color = theme->fptr(data,accuracy);
            KTBS_encoder.rgb_frame->data[0][offset] = ((color)>>16) & 255;
            KTBS_encoder.rgb_frame->data[0][offset+1] = (color>>8) & 255;
            KTBS_encoder.rgb_frame->data[0][offset+2] = (color)& 255;
        }
    }
    //Convert frame RGB to YUV 
    sws_scale(KTBS_encoder.sws_ctx,KTBS_encoder.rgb_frame->data,KTBS_encoder.rgb_frame->linesize,
    0,v_frame.height,KTBS_encoder.yuv_frame->data,KTBS_encoder.yuv_frame->linesize);
    KTBS_encoder.yuv_frame->pts = v_frame.frame_id;
    //Encode
    if(avcodec_send_frame(KTBS_encoder.codec_ctx, KTBS_encoder.yuv_frame) <0){
        printf("Error sending the frame %d\n", v_frame.frame_id);
        exit(1);
        //Error sending the frame
    }
    while(avcodec_receive_packet(KTBS_encoder.codec_ctx,KTBS_encoder.pkt)>=0){
        av_packet_rescale_ts(KTBS_encoder.pkt,KTBS_encoder.codec_ctx->time_base,KTBS_encoder.stream->time_base);
        KTBS_encoder.pkt->stream_index =KTBS_encoder.stream->index;
        av_interleaved_write_frame(KTBS_encoder.fmt_ctx,KTBS_encoder.pkt);
        av_packet_unref(KTBS_encoder.pkt);
    }
}
void flush_encoder(){
    avcodec_send_frame(KTBS_encoder.codec_ctx,NULL);
    while(avcodec_receive_packet(KTBS_encoder.codec_ctx,KTBS_encoder.pkt)>=0){
        av_packet_rescale_ts(KTBS_encoder.pkt,KTBS_encoder.codec_ctx->time_base,KTBS_encoder.stream->time_base);
        KTBS_encoder.pkt->stream_index =KTBS_encoder.stream->index;
        av_interleaved_write_frame(KTBS_encoder.fmt_ctx,KTBS_encoder.pkt);
        av_packet_unref(KTBS_encoder.pkt);
    }
}
void free_encoder(){
    av_write_trailer(KTBS_encoder.fmt_ctx);
    sws_freeContext(KTBS_encoder.sws_ctx);
    av_frame_free(&KTBS_encoder.rgb_frame);
    av_frame_free(&KTBS_encoder.yuv_frame);
    av_packet_free(&KTBS_encoder.pkt);
    avcodec_free_context(&KTBS_encoder.codec_ctx);
    avio_closep(&KTBS_encoder.fmt_ctx->pb);
    avformat_free_context(KTBS_encoder.fmt_ctx);
}