#include<string.h>
#include<stdio.h>

#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
// #include<libavutil/avutil.h>
#include<libavformat/avformat.h>

typedef struct {
    AVDictionary *options;
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec;
    AVPacket *pkt;
    AVFrame *frame;
    int video_stream_index;
} Stream;


AVFormatContext *open_file_stream(Stream *s, const char *filename){
    s->fmt_ctx = NULL;
    s->options = NULL;
    s->dec = NULL;
    s->pkt = NULL;
    s->frame = NULL;

    int ret = avformat_open_input(&s->fmt_ctx, filename, NULL, NULL);
    if (ret<0) return NULL;

    int stream_info = avformat_find_stream_info(&s->fmt_ctx, &s->options);
    if (stream_info<0) {
        avformat_close_input(&s->fmt_ctx);
        return NULL;
    }
    // TODO: stream info, stream init

    return s;
}

AVPacket *read_frame(Stream *s){
    if (s->dec == NULL){ // init dec
        AVCodec *codex = avcodec_find_decoder(s->fmt_ctx->video_codec_id);
        AVCodecParameters *params;
        s->dec = avcodec_alloc_context3(codex);
         if (avcodec_parameters_to_context(s->dec, params)<0){
            avcodec_free_context(&s->dec);
            return NULL;
         }
        avcodec_open2(s->dec, codex, &s->options);
    }
    if (s->pkt == NULL) s->pkt = av_packet_alloc();

    // -- init stuff --
    
}
