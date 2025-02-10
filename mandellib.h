#ifndef MANDELBROT
#define MANDELBROT
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <time.h>
#include "m_theme.h"
typedef struct Frame {int** pixels; int height; int width;M_Theme_Prim* theme;}Frame;
Frame* initFrame(const int width, const int height,M_Theme_Prim* theme);
void createFrame(Frame* frame, int accuracy, const double x_min, const double x_max, const double y_min, const double y_max);
static inline int computeAt(int maxIter,double rp, double ip);
void writeImage(const char* path, Frame* frame);
void directWriteImage(const char* path, int width, int height, int accuracy, double x_min,double x_max, double y_min, double y_max);
void freeFrame(Frame* frame);
#endif