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

// Abstractio for av library
struct VideoDecoder{
    VideoDecoder();
    bool init(const char *filename);
    void quit();
    bool flush_codec();
    bool decode_packet(RgbFrame *frame);
    int read_frame();
    void clear_frame();
    constexpr bool is_video_stream() { return m_packet != NULL && m_packet->stream_index == m_video_stream_index; };
    constexpr uint32_t width() const { return m_av_decoder_ctx->width; }
    constexpr uint32_t height() const { return m_av_decoder_ctx->height; };

    AVFormatContext *m_av_format_ctx;
    AVCodecContext *m_av_decoder_ctx;
    public:

        AVPacket *m_packet;
        AVFrame *m_frame;
        int m_video_stream_index;
        SwsContext *m_sws_scaler_ctx;
};

/*
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
*/
