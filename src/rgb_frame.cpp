#include <libavutil/pixfmt.h>
#include <string.h>
#include <cassert>
#include <stdio.h>
#include "rgb_frame.h"

#define RGB_WIDTH 800
#define RGB_HEIGHT 600

extern "C"{
    #include <libswscale/swscale.h>
}

static SwsContext *yuv420p_to_rgb24;

bool init_rgbframe2(RGBFrame *rgb, int src_w, int src_h, AVPixelFormat src_pix_format){
    memset(rgb->data, 0, sizeof(uint8_t *) * 4);
    memset(rgb->linesizes, 0, sizeof(int) * 4);
    rgb->width = RGB_WIDTH;
    rgb->height = RGB_HEIGHT;

    int ret;
    ret = av_image_alloc(rgb->data, rgb->linesizes, rgb->width, rgb->height, AV_PIX_FMT_RGB24, 1);
    if(ret < 0){
        fprintf(stderr, "Error: failed to allocate rgb buffer");
        return false;
    }

    rgb->buffer_size = ret;
    // initialize sws_context for yuv420p to rgb conversion
    yuv420p_to_rgb24 = sws_getContext(src_w,
                                      src_h,
                                      src_pix_format,
                                      rgb->width,
                                      rgb->height,
                                      AV_PIX_FMT_RGB24,
                                      SWS_BILINEAR, NULL, NULL, NULL);
    if(!yuv420p_to_rgb24){
        fprintf(stderr, "Error: Failed to initialize sws context\n");
        return false;
    }

    return true;
};

bool ppm_write_rgb24_to_file(RGBFrame *rgb, const char *filename){
    assert(rgb != NULL);

    FILE *f = fopen(filename, "w");
    if(!f){
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return false;
    }

    uint8_t *img_buffer = rgb->data[0];
    int pitch = rgb->linesizes[0];
    uint8_t *p;

    fprintf(f, "P3\n%d %d\n255\n", rgb->width, rgb->height);
    for(int y = 0; y < rgb->height; y++){
        for(int x = 0; x < rgb->width; x++){
            p = &img_buffer[y * pitch + x];
            fprintf(f, "%d %d %d\n", *p, *(p + 1), *(p + 2));
            if(ferror(f)){
                fprintf(stderr, "Error: Failed to write to %s\n", filename);
                return false;
            }
        };
    }
    return true;
};

void free_rgbframe(RGBFrame *frame){
    if(frame != NULL){
        av_freep(frame->data[0]);
    }
}

bool convert_yuv420p_to_rgb24(RGBFrame *rgb, AVFrame *yuv_frame){
    int ret;
    ret = sws_scale(yuv420p_to_rgb24, 
                    yuv_frame->data, 
                    yuv_frame->linesize, 
                    0, 
                    yuv_frame->height, 
                    rgb->data, 
                    rgb->linesizes);
    if(ret < 0){
        fprintf(stderr, "Error: failed to convert to rgb\n");
        return false;
    }
    return true;
};
