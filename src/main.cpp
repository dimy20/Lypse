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
#include "rgb_frame.h"
#include "video.h"

RGBFrame rgb_frame;

int main (int argc, char **argv){
    VideoDecodingState decoding_state;

    int ret = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s  input_file video_output_file audio_output_file\n"
                "API example program to show how to read frames from an input file.\n"
                "This program reads frames from a file, decodes them, and writes decoded\n"
                "video frames to a rawvideo file named video_output_file, and decoded\n",
                argv[0]);
        exit(1);
    }

    const char *src_filename = argv[1];

    if(!video_decoding_state_init(&decoding_state, src_filename)){
        exit(1);
    }

    //TODO: change this to a different flow, widht,height and pix_fmt can be 
    //easilly mistaken to be members for rgb whilist in reality they are hints
    //for sws_scale, this should be done differently.
    if(!rgbframe_init(&rgb_frame, decoding_state.width, decoding_state.height, decoding_state.pix_fmt)){
        exit(1);
    }

    /* read frames from the file */
    while (av_read_frame(decoding_state.format_ctx, decoding_state.packet) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        if(decoding_state.packet->stream_index == decoding_state.stream_index){
            //TODO:do not pass packet as arg since is already contained in the state
            ret = video_decode_packet(&decoding_state, decoding_state.packet, &rgb_frame);
        }

        if(ret < 0){
            break;
        }
        av_packet_unref(decoding_state.packet);
    }

    assert(video_decoding_flush(&decoding_state) >= 0);
    video_decoding_state_quit(&decoding_state);
    rgbframe_quit(&rgb_frame);
    return ret < 0;
}
