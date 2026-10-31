#include"file_handler.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

RGB *process_frame(Stream *s){
    int ret = sws_scale(s->sws_ctx, (const uint8_t * const *)s->frame->data, s->frame->linesize, 0, 
    s->frame->height, s->rgb_frame->data, s->rgb_frame->linesize);

    if (ret<=0) return NULL;
    uint64_t r_sum = 0, g_sum = 0, b_sum = 0, total = (uint64_t)s->rgb_frame->height * (uint64_t)s->rgb_frame->width;

    for (int y = 0; y<s->rgb_frame->height; y++){
        for (int x = 0; x<s->rgb_frame->width; x++){
            uint8_t *pixel = s->rgb_frame->data[0] + y * s->rgb_frame->linesize[0] + x * 3;

            r_sum+= pixel[0];
            g_sum+=pixel[1];
            b_sum+=pixel[2];
        }
    }
    RGB *res = malloc(sizeof(RGB));
    if (!res) return NULL;
    res->r = r_sum/total;
    res->b = b_sum/total;
    res->g = g_sum/total;
    return res;

}