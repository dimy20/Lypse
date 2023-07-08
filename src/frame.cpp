#include <cassert>
extern "C"{
#include <libavutil/imgutils.h>
};
#include "frame.h"
#include "error.h"

bool rgb_frame_init(RgbFrame *rgb, int width, int height){
    assert(rgb != NULL);

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                             width,
                                             height,
                                             1);
    LOG_ERROR_RET(num_bytes < 0, "Error: Failed to get buffer size for frame\n");

    rgb->frame_buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    rgb->av_frame = av_frame_alloc();

    LOG_ERROR_RET(!rgb->frame_buffer, "Error: failed to allocate framebuffer for rgb frame.\n");
    LOG_ERROR_RET(!rgb->av_frame, "Error: failed to allocate AVFrame for rgb frame.\n");

    int ret;
    ret = av_image_fill_arrays(rgb->av_frame->data,
                               rgb->av_frame->linesize,
                               rgb->frame_buffer,
                               AV_PIX_FMT_RGB24,
                               width,
                               height,
                               1);
    LOG_ERROR_RET(ret < 0, "Error: Failed to initialize av Frame arrays for rgb frame buffer\n");

    rgb->av_frame->width = width;
    rgb->av_frame->height = height;

    return true;
};

void rgb_frame_quit(RgbFrame* rgb){
    assert(rgb != NULL);
    if(rgb->frame_buffer){
        av_free(rgb->frame_buffer);
    }
    av_frame_free(&rgb->av_frame);
};

bool rgb_frame_save_to_ppm(const RgbFrame *rgb_frame, const char *filename){
    uint8_t *pixels = rgb_frame->av_frame->data[0];
    int pitch = rgb_frame->av_frame->linesize[0];

    int w = rgb_frame->av_frame->width;
    int h = rgb_frame->av_frame->height;


    FILE *f = fopen(filename, "wb");
    FMT_LOG_ERROR_RET(!f, "Error: failed to open %s\n", filename);

    fprintf(f, "P6\n%d %d\n255\n", w, h);

    size_t offset = 0;

    for(int y = 0; y < h; y++){
        fwrite(pixels + offset, 1, w * 3, f);
        offset = pitch * y;

        FMT_LOG_ERROR_RET(ferror(f), "Error: failed to write to %s\n", filename);
    }

    fclose(f);
    return true;
};

/*
bool save_rgb_image_to_ppm(uint8_t *pixels, int pitch, int w, int h, const char *filename){
    FILE *f = fopen(filename, "wb");
    FMT_LOG_ERROR_RET(!f, "Error: failed to open %s\n", filename);

    fprintf(f, "P6\n%d %d\n255\n", w, h);

    size_t offset = 0;
    for(int y = 0; y < h; y++){
        fwrite(pixels + offset, 1, w * 3, f);
        offset = pitch * y;

        FMT_LOG_ERROR_RET(ferror(f), "Error: failed to write to %s\n", filename);
    }

    fclose(f);
    return true;
}
*/
