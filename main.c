#include <stdio.h>

#include "audio.h"

int main(int argc, const char** argv) {
    const char *filename1, *filename2;
    if (argc == 2) {
        // take first and second streams/channels from file
        filename1 = filename2 = argv[1];
    } else if (argc == 3) {
        // take first streams/channels from both files
        filename1 = argv[1];
        filename2 = argv[2];
    } else {
        fprintf(stderr, "Usage: main <filename1> [filename2]\n");
        exit(1);
    }

    audio_info ai1 = get_audio_info(filename1);
    audio_info ai2 = get_audio_info(filename2);
    choose_channels(ai1, ai2);

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

    free_audio_info(ai1);
    if (filename1 != filename2)
        free_audio_info(ai2);

    // example signals for test
    float d1[10] = {0, 0, 1, 0, -1, 0, 0, 0, 0, 0};
    float d2[10] = {0, 0, 0, 0, 1, 0, -1, 0, 0, 0};
    sd1.data = d1;
    sd2.data = d2;
    sd1.size = sd2.size = 10;

    int delta_samples, sample_rate, delta_time;

    delta_samples = get_delta_samples(sd1, sd2);
    sample_rate = sample_rate1;
    delta_time = (1000 * delta_samples) / sample_rate;

    // free_stream_data(sd1);
    // free_stream_data(sd2);

    printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n",
           delta_samples, sample_rate, delta_time);

    return 0;
}