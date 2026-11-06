#include"frame_filter.h"
#include"file_handler.h"


int process_frame(Stream *s, struct RGB *res){
    int ret;
    // rgb frame alloc
    if (!s->rgb_frame){
        s->rgb_frame = av_frame_alloc();
        if (!s->rgb_frame) {fprintf(stderr, "Failed to allocate AVFrame for RGB\n"); return -2;}
        s->rgb_frame->format = AV_PIX_FMT_RGB24;
        s->rgb_frame->width = s->codec_ctx->width;
        s->rgb_frame->height = s->codec_ctx->height;

        ret = av_frame_get_buffer(s->rgb_frame, 0);
        if (ret<0) return_error(ret);
    }

    // SwS Context alloc
    if (!s->sws_ctx) s->sws_ctx = sws_getContext(s->codec_ctx->width, s->codec_ctx->height, s->codec_ctx->pix_fmt, 
    s->codec_ctx->width, s->codec_ctx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR,NULL, NULL, NULL);

    if (!s->sws_ctx) {fprintf(stderr, "Failed to allocate SwSContext\n"); return -2;}

    ret = sws_scale(s->sws_ctx, (const uint8_t * const *)s->frame->data, s->frame->linesize, 0,
    s->frame->height, s->rgb_frame->data, s->rgb_frame->linesize);
    if (ret != s->rgb_frame->height) return 0;

    uint64_t r_sum = 0;
    uint64_t g_sum = 0;
    uint64_t b_sum = 0;

    uint64_t total =(uint64_t)s->rgb_frame->width * (uint64_t)s->rgb_frame->height;


    for (int y = 0; y < s->rgb_frame->height; y++) {
        uint8_t *pixel = s->rgb_frame->data[0] + y * s->rgb_frame->linesize[0];

        for (int x = 0; x < s->rgb_frame->width; x++, pixel += 3) {
            r_sum += pixel[0];
            g_sum += pixel[1];
            b_sum += pixel[2];
        }
    }
    res->r = r_sum/total;
    res->g = g_sum/total;
    res->b = b_sum/total;
    return 1;
}

int calculate_column(const Stream *s, int64_t previous_pts, const struct RGB *rgb, PixelColumn *col, double column_time_duration,
    PixelColumn *next_col){
    // calculate how many columns a frame's RGB values should occupy
    // if return is 1, current PixelColumn is filled and extra "padding" has been passed to next_col
    // dont care if one frame crosses more han 2 columns(duration > 2*column_time_duration - col->total_time)
    const AVStream *stream = s->fmt_ctx->streams[s->video_stream_index];
    int64_t current_pts = s->frame->best_effort_timestamp;

    if (current_pts == AV_NOPTS_VALUE || previous_pts == AV_NOPTS_VALUE) return -1;

    double duration = (current_pts - previous_pts) * av_q2d(stream->time_base);
    if (col->total_time + duration < column_time_duration){

        col->r_sum += rgb->r * duration;
        col->g_sum += rgb->g * duration;
        col->b_sum += rgb->b * duration;

        col->total_time += duration;

        return 0;
    }
    col->r_sum += rgb->r * (column_time_duration - col->total_time);
    col->g_sum += rgb->g * (column_time_duration - col->total_time);
    col->b_sum += rgb->b * (column_time_duration - col->total_time);

    next_col->r_sum = rgb->r * (duration - column_time_duration + col->total_time);
    next_col->g_sum = rgb->g * (duration - column_time_duration + col->total_time);
    next_col->b_sum = rgb->b * (duration - column_time_duration + col->total_time);
    next_col->total_time = duration - column_time_duration + col->total_time;

    col->total_time = column_time_duration;
    return 1;
}