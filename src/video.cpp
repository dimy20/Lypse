
#include <cassert>
#include "video.h"
#include "error.h"

bool video_decoder_init(VideoDecoderState *state, const char *filename){
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
    FMT_LOG_ERROR_RET(!state->av_format_ctx, "failed to allocate format context for %s\n", filename);

    ret = avformat_open_input(&state->av_format_ctx, filename, NULL, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "failed to open %s\n", filename);

    printf("Format %s, duration %ld us\n", state->av_format_ctx->iformat->long_name, state->av_format_ctx->duration);

    ret = avformat_find_stream_info(state->av_format_ctx, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Could not find stream info on %s\n", filename);

    AVStream *video_stream;
    //find video stream
    for(int i = 0; i < state->av_format_ctx->nb_streams; i++){
        video_codec_params = state->av_format_ctx->streams[i]->codecpar;
        if(video_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }


    FMT_LOG_ERROR_RET(video_stream_index == -1, "Could not find video stream on %s\n", filename);

    AVRational t = state->av_format_ctx->streams[video_stream_index]->time_base;
    av_decoder = avcodec_find_decoder(video_codec_params->codec_id);

    FMT_LOG_ERROR_RET(!av_decoder, "could not find video decoder for %s\n.", filename);

    state->av_decoder_ctx = avcodec_alloc_context3(av_decoder);
    FMT_LOG_ERROR_RET(!state->av_decoder_ctx, "Could not allocate video decoder context for %s\n.", filename);

    ret = avcodec_parameters_to_context(state->av_decoder_ctx, video_codec_params);
    FMT_LOG_ERROR_RET(ret < 0, "Failed to initialize video decoder context for %s\n", filename);
    ret = avcodec_open2(state->av_decoder_ctx, av_decoder, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Failed to initialize video decoder context for %s\n", filename);

    state->packet = av_packet_alloc();
    state->frame = av_frame_alloc();
    FMT_LOG_ERROR_RET(!state->packet, "Failed to create video decoding packet for %s\n", filename);
    FMT_LOG_ERROR_RET(!state->frame, "Failed to create video decoding frame for %s\n", filename);

    state->av_decoder_ctx = state->av_decoder_ctx;
    state->av_format_ctx = state->av_format_ctx;
    state->video_stream_index = video_stream_index;

    return true;
};

bool decode_packet(VideoDecoderState *state, RgbFrame *rgb_frame){
    int ret;
    ret = avcodec_send_packet(state->av_decoder_ctx, state->packet);
    LOG_ERROR_RET(ret < 0, "Error while sending a packet to the decoder");

    while(ret >= 0){
        ret = avcodec_receive_frame(state->av_decoder_ctx, state->frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }else if (ret < 0) {
            LOG_ERROR_RET(ret < 0, "Error while receiving a frame from the decoder: %s");
        }

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

        // convert to rgb24
        ret = sws_scale(state->sws_scaler_ctx, 
                        (const uint8_t * const *)state->frame->data, 
                        state->frame->linesize, 
                        0, 
                        state->frame->height, 
                        rgb_frame->av_frame->data, 
                        rgb_frame->av_frame->linesize);
        LOG_ERROR_RET(ret < 0, "Failed to convert frame to rgb24\n");
    }

    return true;
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
};
