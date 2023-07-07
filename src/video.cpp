
#include "video.h"

#include <cassert>
extern int frame_num;

struct Rgb{
    uint8_t *frame_buffer;
    AVFrame *av_frame;
};
Rgb rgb;

void init_rgb_frame(Rgb *rgb, int width, int height){
    assert(rgb != NULL);

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                             width,
                                             height,
                                             1);

    rgb->frame_buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    rgb->av_frame = av_frame_alloc();
    assert(rgb->av_frame != NULL);
    int ret;
    ret = av_image_fill_arrays(rgb->av_frame->data,
                         rgb->av_frame->linesize,
                         rgb->frame_buffer,
                         AV_PIX_FMT_RGB24,
                         width,
                         height,
                         1);
    assert(ret >= 0);

    rgb->av_frame->width = width;
    rgb->av_frame->height = height;

};

void quit_rgb_frame(Rgb *rgb){
    assert(rgb != NULL);
    if(rgb->frame_buffer){
        av_free(rgb->frame_buffer);
    }
    av_frame_free(&rgb->av_frame);
};

int video_decoder_init(VideoDecoderState *state, const char *filename){
    assert(state != NULL);
    state->av_format_ctx = NULL;
    state->av_decoder_ctx = NULL;
    state->packet = NULL;
    state->frame = NULL;
    state->video_stream_index = -1;
    state->sws_scaler_ctx = NULL;

    AVCodecParameters *video_codec_params;
    const AVCodec *av_decoder;

    int video_stream_index = -1;
    int ret;

    state->av_format_ctx = avformat_alloc_context();
    if(!state->av_format_ctx){
        return -1;
    };

    ret = avformat_open_input(&state->av_format_ctx, filename, NULL, NULL);
    assert(ret >= 0);

    printf("Format %s, duration %ld us\n", state->av_format_ctx->iformat->long_name, state->av_format_ctx->duration);

    ret = avformat_find_stream_info(state->av_format_ctx, NULL);
    assert(ret >= 0);

    //find video stream
    for(int i = 0; i < state->av_format_ctx->nb_streams; i++){
        video_codec_params = state->av_format_ctx->streams[i]->codecpar;
        if(video_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }

    if(video_stream_index == -1){
        fprintf(stderr, "Error: Failed to find video stream in %s\n", filename);
        return -1;
    }


    av_decoder = avcodec_find_decoder(video_codec_params->codec_id);
    assert(av_decoder != NULL);

    state->av_decoder_ctx = avcodec_alloc_context3(av_decoder);
    assert(state->av_decoder_ctx != NULL);
    assert(avcodec_parameters_to_context(state->av_decoder_ctx, video_codec_params) >= 0);
    assert(avcodec_open2(state->av_decoder_ctx, av_decoder, NULL) >= 0);

    state->packet = av_packet_alloc();
    state->frame = av_frame_alloc();
    assert(state->packet != NULL);
    assert(state->frame  != NULL);

    state->av_decoder_ctx = state->av_decoder_ctx;
    state->av_format_ctx = state->av_format_ctx;
    state->video_stream_index = video_stream_index;

    init_rgb_frame(&rgb, state->av_decoder_ctx->width, state->av_decoder_ctx->height);

    printf("Initialization completed!\n");
        
    return 0;
};

bool save_rgb_image_to_ppm(uint8_t *pixels, int pitch, int w, int h, const char *filename){
    FILE *f = fopen(filename, "wb");
    if(!f){
        fprintf(stderr, "Error: failed to open %s\n", filename);
        return false;
    }
    fprintf(f, "P6\n%d %d\n255\n", w, h);

    size_t offset = 0;
    for(int y = 0; y < h; y++){
        fwrite(pixels + offset, 1, w * 3, f);
        offset = pitch * y;

        if(ferror(f)){
            fprintf(stderr, "Error: failed to write to %s\n", filename);
            return false;
        }
    }

    fclose(f);
    return true;
}

int decode_packet(VideoDecoderState *state){
    int ret;
    ret = avcodec_send_packet(state->av_decoder_ctx, state->packet);
    if(ret < 0){
        fprintf(stderr, "Error while sending a packet to the decoder: %s", av_err2str(ret));
        return ret;
    }

    char frame_filename[128];
    while(ret >= 0){
        ret = avcodec_receive_frame(state->av_decoder_ctx, state->frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }else if (ret < 0) {
            fprintf(stderr, "Error while receiving a frame from the decoder: %s", av_err2str(ret));
            return ret;
        }

        memset(frame_filename, 0, 128);
        snprintf(frame_filename, 128, "./frames/frame-%d", frame_num);

        if (state->frame->format != AV_PIX_FMT_YUV420P){
            fprintf(stderr, "Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB");
        }

        if(state->sws_scaler_ctx == NULL){
            state->sws_scaler_ctx = sws_getContext(state->frame->width,
                                                         state->frame->height,
                                                         state->av_decoder_ctx->pix_fmt,
                                                         state->frame->width,
                                                         state->frame->height,
                                                         AV_PIX_FMT_RGB24,
                           SWS_BILINEAR, NULL, NULL, NULL);
        }

        if(!state->sws_scaler_ctx){
            fprintf(stderr, "Error: Failed to initialize sws scaler\n");
            return -1;
        }

        ret = sws_scale(state->sws_scaler_ctx, 
                        (const uint8_t * const *)state->frame->data, 
                        state->frame->linesize, 
                        0, 
                        state->frame->height, 
                        rgb.av_frame->data, 
                        rgb.av_frame->linesize);
        assert(ret >= 0);

        bool ok;
        ok = save_rgb_image_to_ppm(rgb.av_frame->data[0],
                       rgb.av_frame->linesize[0], 
                       state->frame->width,
                       state->frame->height,
                       frame_filename);
        if(!ok){
            return -1;
        }
    }

    return 0;

}

// this should be call only once
bool video_decoder_flush_codec(VideoDecoderState *state){
    // signal end of stream
    int ret;
    ret = avcodec_send_packet(state->av_decoder_ctx, NULL);
    return ret >= 0;
}

void video_decoder_state_quit(VideoDecoderState *state){
    avcodec_free_context(&state->av_decoder_ctx);
    avformat_close_input(&state->av_format_ctx);
    sws_freeContext(state->sws_scaler_ctx);
    av_packet_free(&state->packet);
    av_frame_free(&state->frame);
    quit_rgb_frame(&rgb);
};
