#include "video.h"
#include "rgb_frame.h"
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>


static int frame_count = 0;
static bool open_codec_context(VideoDecodingState *state, enum AVMediaType media_type, int * err);

bool video_decoding_state_init(VideoDecodingState *state, const char *filename){
    state->file = NULL; //TODO: do i need this?
    state->format_ctx = NULL;
    state->decoder_ctx = NULL;
    state->stream = NULL;
    state->stream_index = -1;
    state->filename = std::string(filename);

    /* open input file, and allocate format context */
    if(avformat_open_input(&state->format_ctx, filename, NULL, NULL) < 0){
        fprintf(stderr, "Could not open source file %s\n", filename);
        return false;
    }

    /* retrieve stream information */
    if(avformat_find_stream_info(state->format_ctx, NULL) < 0){
        fprintf(stderr, "Could not find stream information\n");
        return false;
        exit(1);
    }

    int err;
    if(!open_codec_context(state, AVMEDIA_TYPE_VIDEO, &err)){
        //TODO: get string for err
        fprintf(stderr, "Error: Failed to open codec context for video stream\n");
        return false;
    }

    state->width = state->decoder_ctx->width;
    state->height = state->decoder_ctx->height;
    state->pix_fmt = state->decoder_ctx->pix_fmt;

    state->frame = av_frame_alloc();

    /* dump input information to stderr */
    av_dump_format(state->format_ctx, 0, state->filename.c_str(), 0);
    if(!state->stream){
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        return false;
    }

    if (!state->frame) {
        fprintf(stderr, "Could not allocate frame\n");
        //*err = (int)AVERROR(ENOMEM);
        return false;
        //goto end;
    }

    state->packet = av_packet_alloc();
    if (!state->packet) {
        fprintf(stderr, "Could not allocate packet\n");
        return false;
        //ret = AVERROR(ENOMEM);
        //goto end;
    }

    return true;
};

static bool open_codec_context(VideoDecodingState *state, enum AVMediaType media_type, int * err){
    int ret, stream_index;
    AVStream *stream;
    const AVCodec *decoder;

    ret = av_find_best_stream(state->format_ctx, media_type, -1, -1, NULL, 0);

    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(media_type), state->filename.c_str());
        return false;
    }

    stream_index = ret;
    stream = state->format_ctx->streams[stream_index];

    //find decoder for the stream
    decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!decoder){
        fprintf(stderr, "Failed to find %s codec\n", av_get_media_type_string(media_type));
        *err = AVERROR(EINVAL);
        return false;
    }

    /* Allocate a codec context for the decoder */
    state->decoder_ctx = avcodec_alloc_context3(decoder);
    if (!state->decoder_ctx) {
        fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(media_type));
        *err = AVERROR(EINVAL);
        return false;
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(state->decoder_ctx, stream->codecpar)) < 0) {
        fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(media_type));
        *err = ret;
        return false;
    }

    /* Init the decoders */
    if ((ret = avcodec_open2(state->decoder_ctx, decoder, NULL)) < 0) {
        fprintf(stderr, "Failed to open %s codec\n",
                av_get_media_type_string(media_type));
        *err = ret;
        return false;
    }

    state->stream_index = stream_index;
    state->stream = stream;

    return true;
};

int video_decode_packet(VideoDecodingState *state, const AVPacket *packet, RGBFrame *rgb_frame){
    int ret = 0;
    // send packet to the decoder for decoding
    ret = avcodec_send_packet(state->decoder_ctx, packet);
    if(ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    while(ret >= 0){
        ret = avcodec_receive_frame(state->decoder_ctx, state->frame);
        if(ret < 0){
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        };

        if(state->decoder_ctx->codec->type == AVMEDIA_TYPE_VIDEO){
            if(frame_count >= 5){
                break;
            }
            if(!convert_yuv420p_to_rgb24(rgb_frame, state->frame)){
                exit(1);
            }

            char filename[128];
            snprintf(filename, 128, "./frames/frame-%d", frame_count);

            fprintf(stderr, "Saving frame: %s\n", filename);
            if(!ppm_write_rgb24_to_file(rgb_frame, filename)){
                exit(1);
            }

            frame_count++;
        }

        av_frame_unref(state->frame);

        //unreachable?
        if(ret < 0){
            return ret;
        }
    };

   return 0;
};

void video_decoding_state_quit(VideoDecodingState *state){
    avcodec_free_context(&state->decoder_ctx);
    avformat_close_input(&state->format_ctx);
    if(state->file){
        fclose(state->file);
    }

    av_packet_free(&state->packet);
    av_frame_free(&state->frame);
};

// this should be call only once
bool video_decoding_flush(VideoDecodingState *state){
    // signal end of stream
    int ret;
    ret = avcodec_send_packet(state->decoder_ctx, NULL);
    return ret >= 0;
}
