
#pragma once

extern "C"{
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
}

// The result after each frame convertion from yuv420p to rgb24 should be kept in this struct.
struct RGBFrame{
    AVFrame *av_frame;
    uint8_t *data[4] = {NULL};
    int linesizes[4];
    int width;
    int height;
    int buffer_size;
};

bool init_rgbframe2(RGBFrame *rgb, int src_w, int src_h, AVPixelFormat src_pix_format);
void free_rgbframe(RGBFrame);
bool ppm_write_rgb24_to_file(RGBFrame *rgb, const char *filename);
bool convert_yuv420p_to_rgb24(RGBFrame *rgb, AVFrame *yuv_frame);
