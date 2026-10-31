#include<string.h>
#include<stdio.h>

#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
// #include<libavutil/avutil.h>
#include<libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>

typedef struct {
    AVDictionary *options;
    AVFormatContext *fmt_ctx; /*    Seperation of different streams, video,audio,subtitles, etc.
                                    fmt_ctx->streams[] contains all AVStream * channels*/
    AVCodecContext *dec; 
    AVPacket *pkt; // data from ONE only stream
    AVFrame *frame;
    int video_stream_index;

    int state_demux_eof;
    int state_decoder_flush;
    int state_packet_pending;

    struct SwsContext *sws_ctx;
    AVFrame *rgb_frame;
} Stream;

void close_stream(Stream *s){
    av_packet_free(&s->pkt);
    av_frame_free(&s->frame);
    avcodec_free_context(&s->dec);
    avformat_close_input(&s->fmt_ctx);
    av_dict_free(&s->options);
    s->video_stream_index = -1;
    s->state_demux_eof = 0;
    s->state_decoder_flush = 0;
    s->state_packet_pending = 0;
    sws_freeContext(s->sws_ctx);
    av_frame_free(&s->rgb_frame);

}
int init_stream(Stream *s, const char *filename){
    s->fmt_ctx = NULL;
    s->options = NULL;
    s->dec = NULL;
    s->pkt = NULL;
    s->frame = NULL;
    s->video_stream_index = -1;

    s->state_demux_eof = 0;
    s->state_decoder_flush = 0;
    s-> state_packet_pending = 0;

    s->sws_ctx = NULL;
    s->rgb_frame = NULL;

    int ret = avformat_open_input(&s->fmt_ctx, filename, NULL, NULL);
    if (ret<0) {close_stream(s); return 0;}

    ret = avformat_find_stream_info(s->fmt_ctx, &s->options);
    if (ret<0) {close_stream(s); return 0;}

    s->video_stream_index = av_find_best_stream(s->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (s->video_stream_index <0) {close_stream(s); return 0;}


    AVStream *stream = s->fmt_ctx->streams[s->video_stream_index];
    const AVCodec *codex = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codex) {close_stream(s); return 0;}

    s->dec = avcodec_alloc_context3(codex);
    if (!s->dec) {close_stream(s); return 0;}

    if (avcodec_parameters_to_context(s->dec, stream->codecpar)<0){close_stream(s); return 0;}

    if (avcodec_open2(s->dec, codex, &s->options)<0) {close_stream(s); return 0;}
    s->pkt = av_packet_alloc();
    s->frame = av_frame_alloc();
    if (!(s->pkt && s->frame)) {close_stream(s); return 0;}

    s->sws_ctx = sws_getContext(s->dec->width, s->dec->height, s->dec->pix_fmt,
            s->dec->width, s->dec->height, AV_PIX_FMT_RGB24,SWS_BILINEAR,NULL, NULL, NULL);

    if (!s->sws_ctx) {close_stream(s); return 0;}

    s->rgb_frame = av_frame_alloc();

    if (!s->rgb_frame) {close_stream(s); return 0;}

    s->rgb_frame->format = AV_PIX_FMT_RGB24;
    s->rgb_frame->width = s->dec->width;
    s->rgb_frame->height = s->dec->height;

    ret = av_frame_get_buffer(s->rgb_frame, 0);

    if (ret < 0) {close_stream(s); return 0;}

    return 1;
}
int flush_decoder(Stream *s){
    // for EOF use
    int ret = avcodec_send_packet(s->dec, NULL);

    if (ret < 0 && ret != AVERROR_EOF) return ret;

    return 0;
}

int read_frame(Stream *s){ // <0 ffmpeg error, 0 EOF, 1 OK
    /* assume stream_init
        avcodec_send_packet:
            receives next packet, containing more than 1 frames maybe? or frame *type*?
        avcodec_receive_frame:
            actually receives the frame data
    */
    int ret;

    __EAGAIN_REPEAT_RECEIVE_FRAME:

    // receive frame from current pkt
    ret = avcodec_receive_frame(s->dec, s->frame);

    if (ret == 0) return 1;
    if (ret == AVERROR_EOF) return 0;
    if (ret != AVERROR(EAGAIN)) return ret;

    if (s->state_demux_eof == 2) return 0;

    if (s->state_demux_eof == 1){
        ret = flush_decoder(s);
        if (ret < 0) return ret;

        s->state_demux_eof = 2;
        goto __EAGAIN_REPEAT_RECEIVE_FRAME;
    }

    if (s->state_packet_pending) goto __REPEAT_SEND_PACKET;

    __REPEAT_READ_PACKET:

    ret = av_read_frame(s->fmt_ctx, s->pkt);

    if (ret == AVERROR_EOF){
        s->state_demux_eof = 1;
        goto __EAGAIN_REPEAT_RECEIVE_FRAME;
    }

    if (ret < 0) return ret;

    if (s->pkt->stream_index != s->video_stream_index){
        av_packet_unref(s->pkt);
        goto __REPEAT_READ_PACKET;
    }

    s->state_packet_pending = 1;

    __REPEAT_SEND_PACKET:

    ret = avcodec_send_packet(s->dec, s->pkt);

    if (ret == AVERROR(EAGAIN)) goto __EAGAIN_REPEAT_RECEIVE_FRAME;

    av_packet_unref(s->pkt);
    s->state_packet_pending = 0;

    if (ret < 0) return ret;

    goto __EAGAIN_REPEAT_RECEIVE_FRAME;
}