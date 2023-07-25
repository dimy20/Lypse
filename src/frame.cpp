#include <cassert>
extern "C"{
#include <libavutil/imgutils.h>
};
#include "frame.h"
#include "error.h"

RgbFrame::RgbFrame(){
    frame_buffer = NULL;
    av_frame = NULL;
}

bool RgbFrame::init(int width, int height){
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                             width,
                                             height,
                                             1);
    LOG_ERROR_RET(num_bytes < 0, "Error: Failed to get buffer size for frame\n");

    frame_buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_frame = av_frame_alloc();

    LOG_ERROR_RET(!frame_buffer, "Error: failed to allocate framebuffer for rgb frame.\n");
    LOG_ERROR_RET(!av_frame, "Error: failed to allocate AVFrame for rgb frame.\n");

    int ret;
    ret = av_image_fill_arrays(av_frame->data,
                               av_frame->linesize,
                               frame_buffer,
                               AV_PIX_FMT_RGB24,
                               width,
                               height,
                               1);
    LOG_ERROR_RET(ret < 0, "Error: Failed to initialize av Frame arrays for rgb frame buffer\n");

    av_frame->width = width;
    av_frame->height = height;
    return true;
};

void RgbFrame::quit(){
    if(frame_buffer){
        av_free(frame_buffer);
    }
    av_frame_free(&av_frame);
};

bool RgbFrame::save_to_ppm(const char *filename){
    uint8_t *pixels = av_frame->data[0];
    int pitch = av_frame->linesize[0];

    int w = av_frame->width;
    int h = av_frame->height;


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
