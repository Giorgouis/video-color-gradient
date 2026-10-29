#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>

int main(){

    printf("%s", av_version_info());
    return 0;
}