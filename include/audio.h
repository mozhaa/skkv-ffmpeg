#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>

#define AUDIO_INBUF_SIZE 20480
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#define AUDIO_REFILL_THRESH 4096

/* Audio info for libav */
typedef struct {
    const char* filename;
    int stream_index;
    int channel_index;
    int bytes_per_sample;

    AVFormatContext* format_context;
    AVCodecParameters* codec_parameters;
    const AVCodec* codec;
    AVCodecContext* codec_context;
} audio_info;

/* Audio stream */
typedef struct {
    double* data;
    size_t size;
    size_t index;
} stream_data;

/* Write data to stream_data */
int sd_write(stream_data* sd, const void* source, int bytes_per_sample);

/* Get audio info object from file */
audio_info get_audio_info(const char* filename);

/* Choose stream+channel from two files (if files are same,
 * choose two different channels, otherwise pick first channel 
 * from both files) */
void choose_channels(audio_info* ai1, audio_info* ai2);

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