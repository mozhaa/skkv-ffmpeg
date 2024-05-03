#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include <stdlib.h>

/* Specific stream from file */
typedef struct {
    const char* filename;
    int stream_index;
} stream_info;

/* Audio stream */
typedef struct {
    float* data;
    size_t size;
} stream_data;

/* Get sample rate of specified stream in audiofile */
int get_sample_rate(stream_info si);

/* Extract audio stream by index from file with filename */
stream_data extract_audio(stream_info si);

/* Get delta in samples, using cross-corellation method */
int get_delta_samples(stream_data sd1, stream_data sd2);

#endif // AUDIO_H_INCLUDED