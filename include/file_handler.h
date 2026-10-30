#ifndef FILE_HANDLER_G_H
#define FILE_HANDLER_G_H

#include<string.h>
#include<stdio.h>
#include<libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include<libavformat/avformat.h>

AVFormatContext *open_file_stream(const char *formatted_filename);

#endif // FILE_HANDLER_G_H
