#include<string.h>
#include<stdio.h>

#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include<libavutil/error.h>
#include <libavutil/pixfmt.h>
// #include<libavutil/avutil.h>
#include<libavformat/avformat.h>
#include <libswscale/swscale.h>


/* macro just cause i know how to do it like this */
static char err[AV_ERROR_MAX_STRING_SIZE];
#define return_error(ret) do { \ 
        av_make_error_string(err, sizeof(err), ret); \
        fprintf(stderr, "FFmpeg err: %s\n", err); \
        return ret; \
} while (0) 

typedef struct {
    AVDictionary *options;
    AVFormatContext *fmt_ctx; /*    Seperation of different streams, video,audio,subtitles, etc.
                                    fmt_ctx->streams[] contains all AVStream * channels*/
    AVCodecContext *codec_ctx; 
    AVCodec *decoder;
    AVPacket *pkt; // data from ONE only stream
    AVFrame *frame;
    int video_stream_index;

} Stream;

int open_file(const char *filename, Stream *s){
    /* 1 OK, < 0 error */
    s->options = NULL;
    s->fmt_ctx = NULL;
    s->codec_ctx = NULL;
    s->decoder = NULL;
    s->pkt = av_packet_alloc();
    s->frame = NULL;
    s->video_stream_index = -1;

    // open file
    int ret = avformat_open_input(&s->fmt_ctx, filename, NULL, NULL);
    if (ret<0) return_error(ret);

    // find streams info
    ret = avformat_find_stream_info(s->fmt_ctx, &s->options);
    if (ret<0) return_error(ret);

    // find best video stream and stream's decoder
    ret = av_find_best_stream(s->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &s->decoder, 0);
    if (ret<0) return_error(ret);
    if (!s->decoder) {fprintf(stderr, "Decoder not found while av_find_best_stream returned >0\n"); return -1;}
    s->video_stream_index = ret;

    // allocate the decoder context?
    s->codec_ctx = avcodec_alloc_context3(s->decoder);
    if (!s->codec_ctx) return -1;

    // fill decoder context
    AVStream *video_stream = s->fmt_ctx->streams[s->video_stream_index];
    ret = avcodec_parameters_to_context(s->codec_ctx, video_stream->codecpar);
    if (ret<0) {avcodec_free_context(s->codec_ctx); return_error(ret);}

    // open decoder
    ret = avcodec_open2(s->codec_ctx, s->decoder, NULL);
    if (ret<0) {avcodec_free_context(s->codec_ctx); return_error(ret);}

    return 1;
}

int next_packet(Stream *s){

    int ret = av_read_frame(s->fmt_ctx, s->pkt);
    if (ret<0) return_error(ret);

    if (s->pkt->stream_index != s->video_stream_index){
        av_packet_unref(s->pkt);
        return next_packet(s);
    }

    ret = avcodec_send_packet(s->fmt_ctx, s->pkt);
    if (ret<0){
        
    }
    return 1;
}