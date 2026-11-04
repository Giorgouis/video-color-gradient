#ifndef FILE_HANDLER_G_H
#define FILE_HANDLER_G_H

#include<stdio.h>

#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include<libavutil/error.h>
// #include<libavutil/avutil.h>
#include<libavformat/avformat.h>
#include <libswscale/swscale.h>

static char err[AV_ERROR_MAX_STRING_SIZE];

#define return_error(ret) do { \
        av_make_error_string(err, sizeof(err), ret); \
        fprintf(stderr, "FFmpeg err: %s\n", err); \
        return ret; \
} while (0)

typedef struct Stream{
    AVDictionary *options;
    AVFormatContext *fmt_ctx; 
    AVCodecContext *codec_ctx; 
    const AVCodec *decoder;
    AVPacket *pkt;
    AVFrame *frame;
    int video_stream_index;

    int packet_pending;
    int flush;
    int flushed;

} Stream;


int open_file(const char *filename, Stream *s);
int next_packet(Stream *s);
int send_packet(Stream *s);
int read_packet(Stream *s);
int read_frame(Stream *s);

#endif // FILE_HANDLER_G_H
