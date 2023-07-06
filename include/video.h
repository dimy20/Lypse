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

#include <string>
#include "rgb_frame.h"

struct VideoDecodingState{
    AVFormatContext *format_ctx;
    AVCodecContext *decoder_ctx = NULL;

    int width;
    int height;
    enum AVPixelFormat pix_fmt;

    FILE *file;
    AVStream *stream;
    int stream_index;

    std::string filename;
    AVPacket *packet;
    AVFrame *frame; // This will store the result output of decoding a packet
};

bool video_decoding_state_init(VideoDecodingState *state, const char *filename);
void video_decoding_state_quit(VideoDecodingState *state);
int video_decode_packet(VideoDecodingState *state, const AVPacket *packet, RGBFrame *rgb_frame);
bool video_decoding_flush(VideoDecodingState *state);
