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

#include "SDL_events.h"
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
#include "frame.h"
#include "display.h"

const char *filename = "./assets/dummy.mp4";
int frame_num = 0;
bool running = true;
static double time_base;

static Display display;
static VideoDecoder video_decoder;
static RgbFrame frame;

bool init_all(){

    if(!video_decoder.init(filename)){
        video_decoder.quit();
        return false;
    }

    if(!frame.init(video_decoder.width(), video_decoder.height())){
        return false;
    }

    if(!display.init(frame.av_frame->width,
                     frame.av_frame->height,
                     "test"))
        return false;

    if(!video_decoder.time_base(&time_base)){
        fprintf(stderr, "Error: Failed to retrive video stream timebase\n");
        return false;
    }

    return true;
};

void do_input(){
    SDL_Event e;
    while(SDL_PollEvent(&e) > 0){
        if(e.type == SDL_QUIT){
            running = false;
        }
    }
};

int main(int argc, char **argv){
    int ret;
    if(!init_all())
        exit(1);

    while(running){
        do_input();

        while(video_decoder.read_frame() >= 0){
            if(video_decoder.is_video_stream()){
                ret = video_decoder.decode_packet(&frame);
                if(ret < 0){
                    break;
                }

                //Present frame at correct time based on PTS and timebase.
                double pts_seconds = static_cast<double>(frame.av_frame->pts) * time_base;
                double curr_time_seconds = static_cast<double>(SDL_GetTicks()) / 1000.0f;
                if(pts_seconds > curr_time_seconds){
                    double wait_seconds = pts_seconds - curr_time_seconds;
                    SDL_Delay(1000 * wait_seconds);
                }

                display.update_pixels(frame.frame_buffer);
                display.present_pixels();

            }
            video_decoder.clear_frame();
            break;
        }
    }

    video_decoder.flush_codec();
    video_decoder.quit();
    frame.quit();
    display.quit();
    return 0;
}
