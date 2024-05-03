#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>

/* Specific stream info from file */
typedef struct {
    const char* filename;
    int stream_index;
} stream_info;

/* Audio info for libav */
typedef struct {
    int stream_index;

    AVFormatContext* format_context;
    AVCodecParameters* codec_parameters;
    const AVCodec* codec;
    AVCodecContext* codec_context;
    AVStream* stream;
} audio_info;

/* Audio stream */
typedef struct {
    float* data;
    size_t size;
} stream_data;

/* Get audio info object from file */
audio_info get_audio_info(stream_info si);

/* Get sample rate of specified stream in audiofile */
int get_sample_rate(audio_info ai);

/* Extract audio stream by index from file with filename */
stream_data extract_audio(audio_info ai);

/* Get delta in samples, using cross-corellation method */
int get_delta_samples(stream_data sd1, stream_data sd2);

/* Free stream data after usage */
void free_stream_data(stream_data sd);

/* Free audio info after usage */
void free_audio_info(audio_info ai);

#endif // AUDIO_H_INCLUDED