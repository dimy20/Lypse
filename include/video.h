#pragma once

extern "C"{
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "frame.h"

struct VideoDecoderState{
    AVFormatContext *av_format_ctx;
    AVCodecContext *av_decoder_ctx;

    AVPacket *packet;
    AVFrame *frame;
    int video_stream_index;

    SwsContext *sws_scaler_ctx;

};

bool video_decoder_init(VideoDecoderState *state, const char *filename);
bool decode_packet(VideoDecoderState *video_state, RgbFrame *frame);
void video_decoder_state_quit(VideoDecoderState *state);
bool video_decoder_flush_codec(VideoDecoderState *state);
