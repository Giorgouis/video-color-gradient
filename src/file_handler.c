#include "file_handler.h"
#include<stdio.h>

#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include<libavutil/error.h>
// #include<libavutil/avutil.h>
#include<libavformat/avformat.h>
#include <libswscale/swscale.h>


/* macro just cause i know how to do it like this */
static char err[AV_ERROR_MAX_STRING_SIZE];


int open_file(const char *filename, Stream *s){
    /* 1 OK, < 0 error */
    s->options = NULL;
    s->fmt_ctx = NULL;
    s->codec_ctx = NULL;
    s->decoder = NULL;
    s->pkt = av_packet_alloc();
    s->frame = av_frame_alloc();
    if (!s->pkt || !s->frame) {
        fprintf(stderr, "Failed to alloc packet and/or frame\n");
        return -1;
    }
    s->video_stream_index = -1;
    s->packet_pending = 0;
    s->flush = 0;
    s->flushed = 0;

    s->sws_ctx = NULL;
    s->rgb_frame = NULL;

    // open file
    int ret = avformat_open_input(&s->fmt_ctx, filename, NULL, NULL);
    if (ret<0) return_error(ret);

    // find streams info
    ret = avformat_find_stream_info(s->fmt_ctx, NULL);
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
    if (ret<0) {avcodec_free_context(&s->codec_ctx); return_error(ret);}

    // open decoder
    ret = avcodec_open2(s->codec_ctx, s->decoder, NULL);
    if (ret<0) {avcodec_free_context(&s->codec_ctx); return_error(ret);}

    return 1;
}

int next_packet(Stream *s){
    int ret;

    while (1){
        av_packet_unref(s->pkt);

        ret = av_read_frame(s->fmt_ctx, s->pkt);
        if (ret == AVERROR_EOF){ // must send NULL?
            s->flush = 1;
            return ret;
        }

        if (ret<0) return_error(ret);

        if (s->pkt->stream_index == s->video_stream_index) return 0; // if != read next packet
    }
}

int send_packet(Stream *s){
    int ret;
    if (s->flush) {
        ret = avcodec_send_packet(s->codec_ctx, NULL);
        if (ret == 0) s->flushed = 1;

        return ret;
    }

     return avcodec_send_packet(s->codec_ctx, s->pkt);
}

int read_packet(Stream *s){
    av_frame_unref(s->frame);
    return avcodec_receive_frame(s->codec_ctx, s->frame);
}

int read_frame(Stream *s){
    int ret;
    while (1){
        ret = read_packet(s);

        if (ret == 0) return 1; // read frame, no need for anything else

        if (ret == AVERROR_EOF) return 0; // finished

        if (ret != AVERROR(EAGAIN)) return_error(ret); // something wrong

        if (!s->packet_pending && !s->flush){
            ret = next_packet(s);

            if (ret == AVERROR_EOF) continue; // will get NULL next time

            if (ret<0) return_error(ret); // something wrong

            s->packet_pending = 1; // we need next packet

        }
        if (s->flushed){
            return AVERROR_BUG; // idk if flushed it shouldnt reach here
        }
        ret = send_packet(s);

        if (ret == 0){
            if (!s->flush){
                av_packet_unref(s->pkt);
                s->packet_pending = 0;
            }
            continue;
        }
        
        if (ret == AVERROR(EAGAIN)) continue;

        if (ret == AVERROR_EOF) return 0; // actually finished

        return_error(ret);
        
    }
}

