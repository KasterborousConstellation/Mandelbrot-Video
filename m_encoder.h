#ifndef MANDEL_ENCODER

#define MANDEL_ENCODER
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
struct M_Encoder {
    AVFormatContext* fmt_ctx ;
    AVCodecContext* codec_ctx;
    AVStream* stream;
    AVCodec* codec;
    AVFrame* rgb_frame;
    AVFrame* yuv_frame;
    AVPacket* pkt;
    struct SwsContext* sws_ctx;
};
typedef struct Virtual_Frame {int width; int height; int frame_id;} Virtual_Frame;
void init_encoder(int width, int height, int fps,char* filename, char* path);
void encode(Virtual_Frame v_frame, int* pixels);
void flush_encoder();
void free_encoder();

#endif