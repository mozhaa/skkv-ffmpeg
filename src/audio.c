#include "audio.h"

#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>

#include <fftw3.h>

int get_sample_rate(stream_info si) {
    return 50000;
}

stream_data extract_audio(stream_info si) {
    stream_data sd;
    sd.data = malloc(100);
    sd.size = 100;
    return sd;
}

int get_delta_samples(stream_data sd1, stream_data sd2) {
    return 3500;
}