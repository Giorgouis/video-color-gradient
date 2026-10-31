#ifndef FRAME_FILTER_G_H
#define FRAME_FILTER_G_H

#include"file_handler.h"

typedef struct RGB{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

RGB *process_frame(Stream *s);

#endif // FRAME_FILTER_G_H