#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>

#include <fftw3.h>

int main(int argc, char** argv) {
    char* filename1, filename2;
    int stream_index1, stream_index2;
    if (argc == 2) {
        // take first and second stream from file
        filename1 = filename2 = argv[1];
        stream_index1 = 0;
        stream_index2 = 1;
    } else if (argc == 3) {
        // take first streams from both files
        filename1 = argv[1];
        filename2 = argv[2];
        stream_index1 = stream_index2 = 0;
    } else {
        fprintf(stderr, "Usage: delta <filename1> [filename2]\n");
        exit(1);
    }



    int delta_samples = 0, sample_rate = 0, delta_time = 0;

    printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n",
           delta_samples, sample_rate, delta_time);

    return 0;
}