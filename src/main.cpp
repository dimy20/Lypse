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
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include "video.h"

#define MAX_FRAMES_NUM 1
const char *filename = "./assets/dummy.mp4";
int frame_num = 0;

int main (int argc, char **argv){
    VideoDecoderState video_decoder;

    if(video_decoder_init(&video_decoder, filename) < 0){
        return -1;
    }

    int ret;
    while(av_read_frame(video_decoder.av_format_ctx, video_decoder.packet) >= 0){
        if(video_decoder.packet->stream_index == video_decoder.video_stream_index){
            ret = decode_packet(&video_decoder);
            if(ret < 0){
                break;
            }
            if(++frame_num >= MAX_FRAMES_NUM){
                break;
            }
        }
        av_packet_unref(video_decoder.packet);
    }

    video_decoder_flush_codec(&video_decoder);
    video_decoder_state_quit(&video_decoder);
    return 0;
}
