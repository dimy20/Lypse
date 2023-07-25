#include <cassert>
#include "video.h"
#include "error.h"

VideoDecoder::VideoDecoder(){
    m_av_format_ctx = NULL;
    m_av_decoder_ctx = NULL;
    m_packet = NULL;
    m_frame = NULL;
    m_video_stream_index = -1;
    m_sws_scaler_ctx = NULL;
};

bool VideoDecoder::init(const char *filename){
    AVCodecParameters *video_codec_params;
    const AVCodec *av_decoder;
    int ret;
    m_av_format_ctx = avformat_alloc_context();
    FMT_LOG_ERROR_RET(!m_av_format_ctx, "failed to allocate format context for %s\n", filename);

    ret = avformat_open_input(&m_av_format_ctx, filename, NULL, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "failed to open %s\n", filename);

    printf("Format %s, duration %ld us\n", m_av_format_ctx->iformat->long_name, m_av_format_ctx->duration);

    ret = avformat_find_stream_info(m_av_format_ctx, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Could not find stream info on %s\n", filename);

    //find video stream
    for(int i = 0; i < m_av_format_ctx->nb_streams; i++){
        video_codec_params = m_av_format_ctx->streams[i]->codecpar;
        if(video_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            m_video_stream_index = i;
            break;
        }
    }
    FMT_LOG_ERROR_RET(m_video_stream_index == -1, "Could not find video stream on %s\n", filename);
    av_decoder = avcodec_find_decoder(video_codec_params->codec_id);

    FMT_LOG_ERROR_RET(!av_decoder, "could not find video decoder for %s\n.", filename);

    m_av_decoder_ctx = avcodec_alloc_context3(av_decoder);
    FMT_LOG_ERROR_RET(!m_av_decoder_ctx, "Could not allocate video decoder context for %s\n.", filename);

    ret = avcodec_parameters_to_context(m_av_decoder_ctx, video_codec_params);
    FMT_LOG_ERROR_RET(ret < 0, "Failed to initialize video decoder context for %s\n", filename);
    ret = avcodec_open2(m_av_decoder_ctx, av_decoder, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Failed to initialize video decoder context for %s\n", filename);

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();
    FMT_LOG_ERROR_RET(!m_packet, "Failed to create video decoding packet for %s\n", filename);
    FMT_LOG_ERROR_RET(!m_frame, "Failed to create video decoding frame for %s\n", filename);

    return true;
};

int VideoDecoder::read_frame(){ return av_read_frame(m_av_format_ctx, m_packet); }

void VideoDecoder::clear_frame() { av_packet_unref(m_packet); };

bool VideoDecoder::decode_packet(RgbFrame *rgb_frame){
    int ret;
    ret = avcodec_send_packet(m_av_decoder_ctx, m_packet);
    LOG_ERROR_RET(ret < 0, "Error while sending a packet to the decoder");

    while(ret >= 0){
        ret = avcodec_receive_frame(m_av_decoder_ctx, m_frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }else if (ret < 0) {
            LOG_ERROR_RET(ret < 0, "Error while receiving a frame from the decoder: %s");
        }

        if (m_frame->format != AV_PIX_FMT_YUV420P){
            fprintf(stderr, "Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB");
        }

        if(m_sws_scaler_ctx == NULL){
            m_sws_scaler_ctx = sws_getContext(m_frame->width,
                                              m_frame->height,
                                              m_av_decoder_ctx->pix_fmt,
                                              m_frame->width,
                                              m_frame->height,
                                              AV_PIX_FMT_RGB24,
                                              SWS_BILINEAR, NULL, NULL, NULL);
        }

        if(!m_sws_scaler_ctx){
            fprintf(stderr, "Error: Failed to initialize sws scaler\n");
            return false;
        }

        // convert to rgb24
        ret = sws_scale(m_sws_scaler_ctx, 
                        (const uint8_t * const *)m_frame->data, 
                        m_frame->linesize, 
                        0, 
                        m_frame->height, 
                        rgb_frame->av_frame->data, 
                        rgb_frame->av_frame->linesize);
        LOG_ERROR_RET(ret < 0, "Failed to convert frame to rgb24\n");
    }

    return true;

}

void VideoDecoder::quit(){
    avcodec_free_context(&m_av_decoder_ctx);
    avformat_close_input(&m_av_format_ctx);
    sws_freeContext(m_sws_scaler_ctx);
    av_packet_free(&m_packet);
    av_frame_free(&m_frame);
}

bool VideoDecoder::flush_codec(){
    // signal end of stream
    int ret;
    ret = avcodec_send_packet(m_av_decoder_ctx, NULL);
    return ret >= 0;
}
