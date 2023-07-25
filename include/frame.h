#pragma once

extern "C"{
    #include <libavcodec/avcodec.h>
};

struct RgbFrame{
    RgbFrame();
    bool init(int width, int height);
    void quit();
    bool save_to_ppm(const char *filename);
    uint8_t *frame_buffer;
    AVFrame *av_frame;
};
