#ifndef FILE_HANDLER_G_H
#define FILE_HANDLER_G_H

#include<string.h>
#include<stdio.h>
#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include<libavformat/avformat.h>

typedef struct Stream {
    AVDictionary *options;
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec; 
    AVPacket *pkt;
    AVFrame *frame;
    int video_stream_index;

    int state_demux_eof;
    int state_decoder_flush;
    int state_packet_pending;
} Stream;

void close_stream(Stream *s);
int init_stream(Stream *s, const char *filename);
int flush_decoder(Stream *s);
int read_frame(Stream *s);

#endif // FILE_HANDLER_G_H
