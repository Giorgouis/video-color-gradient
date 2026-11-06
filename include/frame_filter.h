#ifndef FRAME_FILTER_G_H
#define FRAME_FILTER_G_H


#include<png.h>
#include<libswscale/swscale.h>
#include <libavutil/frame.h>

struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct PixelColumn{
    double r_sum;
    double g_sum;
    double b_sum;
    double total_time;
} PixelColumn;

int process_frame(Stream *s, struct RGB *res);
int calculate_column(const Stream *s, int64_t previous_pts, const struct RGB *rgb, PixelColumn *col, double column_time_duration,
    PixelColumn *next_col);
#endif