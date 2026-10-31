#include"file_handler.h"
#include"frame_filter.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Stream s;

    if (!init_stream(&s, argv[1])) {
        fprintf(stderr, "Failed to open input file.\n");
        return 1;
    }

    int ret;
    int frame_number = 0;
    RGB *rgb;

    while ((ret = read_frame(&s)) > 0) {
        frame_number++;

        printf("Frame %d: ", frame_number);
        rgb = process_frame(&s);
        if (!rgb) {
            fprintf(stderr, "Failed to process frame.\n");
            close_stream(&s);
            free(rgb);
            return 1;
        }

        printf("RGB(%u %u %u)\n", (unsigned)rgb->r, (unsigned)rgb->g, (unsigned)rgb->b);
        free(rgb);
    }

    if (ret < 0)
        fprintf(stderr, "Decoding error: %d\n", ret);

    close_stream(&s);
    return ret < 0 ? 1 : 0;
}