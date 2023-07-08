#pragma once

extern "C"{
    #include <libavcodec/avcodec.h>
};

struct RgbFrame{
    uint8_t *frame_buffer;
    AVFrame *av_frame;
};

bool rgb_frame_init(RgbFrame *rgb, int width, int height);
void rgb_frame_quit(RgbFrame *rgb);
bool rgb_frame_save_to_ppm(const RgbFrame *rgb_frame, const char *filename);
