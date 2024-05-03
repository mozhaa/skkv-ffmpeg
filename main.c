#include <stdio.h>

#include "audio.h"

int main(int argc, char** argv) {
    stream_info si1, si2;
    if (argc == 2) {
        // take first and second stream from file
        si1.filename = si2.filename = argv[1];
        si1.stream_index = 0;
        si2.stream_index = 1;
    } else if (argc == 3) {
        // take first streams from both files
        si1.filename = argv[1];
        si2.filename = argv[2];
        si1.stream_index = si2.stream_index = 0;
    } else {
        fprintf(stderr, "Usage: delta <filename1> [filename2]\n");
        exit(1);
    }

    audio_info ai1 = get_audio_info(si1), ai2 = get_audio_info(si2);

    int sample_rate1 = get_sample_rate(ai1);
    int sample_rate2 = get_sample_rate(ai2);
    if (sample_rate1 != sample_rate2) {
        fprintf(stderr,
                "Different sample rate streams are not supported (%d != "
                "%d)\n",
                sample_rate1, sample_rate2);
        exit(2);
    }

    stream_data sd1 = extract_audio(ai1);
    stream_data sd2 = extract_audio(ai2);

    float d1[10] = {0, 0, 1, 0, -1, 0, 0, 0, 0, 0};
    float d2[10] = {0, 0, 0, 0, 1, 0, -1, 0, 0, 0};
    sd1.data = d1;
    sd2.data = d2;
    sd1.size = sd2.size = 10;

    int delta_samples, sample_rate, delta_time;

    delta_samples = get_delta_samples(sd1, sd2);
    sample_rate = sample_rate1;
    delta_time = (1000 * delta_samples) / sample_rate;

    printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n",
           delta_samples, sample_rate, delta_time);

    return 0;
}