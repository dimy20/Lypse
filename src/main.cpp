/*
 * Copyright (c) 2012 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file libavformat and libavcodec demuxing and decoding API usage example
 * @example demux_decode.c
 *
 * Show how to use the libavformat and libavcodec API to demux and decode audio
 * and video data. Write the output as raw audio and input files to be played by
 * ffplay.
 */



#include <cstdio>
#include <cassert>
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
#include <iostream>

//CONVERTION TO RGB
#define DEST_WIDTH 600
#define DEST_HEIGHT 400
#define DEST_BUFFER_SIZE DEST_WIDTH * DEST_HEIGHT * 3 // RGB24 requires 3 bytes per pixel, RGBA requires 4 bytes per pixel
uint8_t *rgb_buffer;
AVFrame *rgb_frame;

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static int width, height;
static enum AVPixelFormat pix_fmt;
static AVStream *video_stream = NULL;
static const char *src_filename = NULL;
static const char *video_dst_filename = NULL;
static FILE *video_dst_file = NULL;

static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static int video_dst_bufsize;

static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket *pkt = NULL;
static int video_frame_count = 0;
static SwsContext *sws_context;




static int frame_count = 0;
void convert_yuv420p_to_rgb24(const AVFrame *frame);
void save_current_rgbframe_to_ppm();

static int output_video_frame(AVFrame *frame){
    if (frame->width != width || frame->height != height ||
        frame->format != pix_fmt) {
        /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
        fprintf(stderr, "Error: Width, height and pixel format have to be "
                "constant in a rawvideo file, but the width, height or "
                "pixel format of the input video changed:\n"
                "old: width = %d, height = %d, format = %s\n"
                "new: width = %d, height = %d, format = %s\n",
                width, height, av_get_pix_fmt_name(pix_fmt),
                frame->width, frame->height,
                av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)));
        return -1;
    }

    printf("video_frame n:%d\n",
           video_frame_count++);

    const char * format_name = av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format));
    std::cout << "Format name : " << format_name << "\n";
    /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
    av_image_copy(video_dst_data, video_dst_linesize,
                  (const uint8_t **)(frame->data), frame->linesize,
                  pix_fmt, width, height);

    /* write to rawvideo file */
    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
    return 0;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *pkt){
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }

        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO){
            ret = output_video_frame(frame);
            convert_yuv420p_to_rgb24(frame);
            save_current_rgbframe_to_ppm();
        }


        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }

    return 0;
}

void save_current_rgbframe_to_ppm(){
    char filename[128];
    snprintf(filename, 128, "./frames/frame-%d", frame_count);
    FILE *f = fopen(filename, "w");
    assert(f != NULL);

    uint8_t r, g, b;
    size_t pitch = rgb_frame->linesize[0];

    fprintf(f,"P3\n%d %d\n255\n", DEST_WIDTH, DEST_HEIGHT);
    for(int y = 0; y < DEST_HEIGHT; y++){
        for(int x = 0; x < DEST_WIDTH; x++){
            uint8_t * p = &rgb_buffer[y * pitch + x];

            r = *p;
            g = *(p + 1);
            b = *(p + 2);
            fprintf(f, "%d %d %d\n", r, g, b);
        }
    }
    frame_count++;
};

void convert_yuv420p_to_rgb24(const AVFrame *frame){
    int ret;
    ret = sws_scale(sws_context, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);
    if(ret < 0){
        std::cerr << "yuv420p to rgb conversion failed\n";
        exit(1);
    }
};

static int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

bool init_rgb_frame(){
    int ret;
    rgb_frame = av_frame_alloc();

    if(!rgb_frame){
        std::cerr << "Error: Failed to create rgb frame";
    }

    // It is required that this fields are initialized before
    // a call to av_frame_get_buffer;
    rgb_frame->width = DEST_WIDTH;
    rgb_frame->height = DEST_HEIGHT;
    rgb_frame->format = AV_PIX_FMT_RGB24;

    if(av_frame_get_buffer(rgb_frame, 0) < 0 ){
        std::cerr << "Error: failed to get buffer for rgb frame\n";
        return false;
    }
    int rgb_bufsize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, DEST_WIDTH, DEST_HEIGHT, 1);
    if(rgb_bufsize < 0){
        std::cerr << "Error: failed to caculate rgb buffer size\n";
        return false;
    }

    rgb_buffer = static_cast<uint8_t *>(av_malloc(sizeof(uint8_t) * rgb_bufsize));
    if(!rgb_buffer){
        std::cerr << "Error: Failed to allocate memory for rgb buffer\n";
        return false;
    }

    ret = av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24, DEST_WIDTH, DEST_HEIGHT, 1);
    if(ret < 0){
        std::cerr << "Error: Failed to fill rgb image buffers\n";
        return false;
    }

    return true;
};

int main (int argc, char **argv){
    //buffer[(y * w + x) * 3]
    int ret = 0;

    if(!init_rgb_frame()){
        exit(1);
    }

    if (argc != 3) {
        fprintf(stderr, "usage: %s  input_file video_output_file audio_output_file\n"
                "API example program to show how to read frames from an input file.\n"
                "This program reads frames from a file, decodes them, and writes decoded\n"
                "video frames to a rawvideo file named video_output_file, and decoded\n",
                argv[0]);
        exit(1);
    }

    src_filename = argv[1];
    video_dst_filename = argv[2];

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }
    //sws_scale context for parsing yuv420p to rgb

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];

        video_dst_file = fopen(video_dst_filename, "wb");
        if (!video_dst_file) {
            fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
            ret = 1;
            goto end;
        }

        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            goto end;
        }
        video_dst_bufsize = ret;
    }

    sws_context = sws_getContext(width, height, AV_PIX_FMT_YUV420P, DEST_WIDTH, DEST_HEIGHT, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    if(!sws_context){
        std::cerr << "Error: Failed to create sws context\n";
        exit(1);
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if(!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (video_stream)
        printf("Demuxing video from file '%s' into '%s'\n", src_filename, video_dst_filename);
    /* read frames from the file */
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt->stream_index == video_stream_idx)
            ret = decode_packet(video_dec_ctx, pkt);

        av_packet_unref(pkt);
        if (ret < 0)
            break;
    }

    /* flush the decoders */
    if (video_dec_ctx)
        decode_packet(video_dec_ctx, NULL);

    printf("Demuxing succeeded.\n");

    if (video_stream) {
        printf("Play the output video file with the command:\n"
               "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
               av_get_pix_fmt_name(pix_fmt), width, height,
               video_dst_filename);
    }

end:
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    if (video_dst_file)
        fclose(video_dst_file);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_free(video_dst_data[0]);

    return ret < 0;
}
