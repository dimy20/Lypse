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

struct VideoDecoderState{
    AVFormatContext *av_format_ctx;
    AVCodecContext *av_decoder_ctx;

    AVPacket *packet;
    AVFrame *frame;
    int video_stream_index;

    SwsContext *sws_scaler_ctx;

};

int video_decoder_init(VideoDecoderState *video_state, const char *filename);
int decode_packet(VideoDecoderState *video_state);
void video_decoder_state_quit(VideoDecoderState *state);
bool video_decoder_flush_codec(VideoDecoderState *state);
