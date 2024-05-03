#include "audio.h"

#include <stdio.h>
#include <stdlib.h>

#include <complex.h>
#include <fftw3.h>

#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>

void _log(const char* fmt, ...) {
    va_list l;
    va_start(l, fmt);
    printf("\033[1;31m[LOG]:\033[0m ");
    vprintf(fmt, l);
    printf("\n");
    va_end(l);
}

static int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx,
                              AVFormatContext* fmt_ctx, enum AVMediaType type) {
    int ret, stream_index;
    AVStream* st;
    const AVCodec* dec = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file\n",
                av_get_media_type_string(type));
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr,
                    "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

static void find_first_channel(audio_info ai) {
    for (int i = 0; i < ai.format_context->nb_streams; ++i) {
        AVStream* stream = ai.format_context->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            // audio stream
            _log("Found audio stream #%d, nb_channels=%d", i,
                 codecpar->ch_layout.nb_channels);
            if (codecpar->ch_layout.nb_channels > 0) {
                // pick this channel
                ai.stream_index = i;
                ai.channel_index = 0;
                return;
            }
        }
    }
    fprintf(stderr, "File \"%s\" contains less than 2 channels\n", ai.filename);
    exit(1);
}

void choose_channels(audio_info ai1, audio_info ai2) {
    ai1.stream_index = ai2.stream_index = -1;
    if (!strcmp(ai1.filename, ai2.filename)) {
        // if these are same files
        // find two different channels from this file
        for (int i = 0; i < ai1.format_context->nb_streams; ++i) {
            AVStream* stream = ai1.format_context->streams[i];
            AVCodecParameters* codecpar = stream->codecpar;
            if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                // audio stream
                _log("Found audio stream #%d, nb_channels=%d", i,
                     codecpar->ch_layout.nb_channels);
                if (ai1.stream_index != -1 &&
                    codecpar->ch_layout.nb_channels > 0) {
                    // pick first channel for sure
                    ai1.stream_index = i;
                    ai1.channel_index = 0;
                    if (codecpar->ch_layout.nb_channels > 1) {
                        // pick these two channels
                        ai2.stream_index = i;
                        ai2.channel_index = 1;
                        return;
                    }
                } else if (codecpar->ch_layout.nb_channels > 0) {
                    // pick first channel from this stream
                    ai2.stream_index = i;
                    ai2.channel_index = 0;
                    return;
                }
            }
        }
        fprintf(stderr, "File \"%s\" contains less than 2 channels\n",
                ai1.filename);
        exit(1);
    } else {
        // if these are different files
        // pick first channel from both files
        find_first_channel(ai1);
        find_first_channel(ai2);
    }
}

audio_info get_audio_info(const char* filename) {
    audio_info ai;
    ai.filename = filename;

    if ((ai.format_context = avformat_alloc_context()) == NULL) {
        fprintf(stderr, "Could not allocate format context\n");
        exit(1);
    }
    if (avformat_open_input(&ai.format_context, filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input for format context\n");
        exit(1);
    }
    if (avformat_find_stream_info(ai.format_context, NULL) < 0) {
        fprintf(stderr, "Could not find stream info\n");
        exit(1);
    }

    if (open_codec_context(&ai.stream_index, &ai.codec_context,
                           ai.format_context, AVMEDIA_TYPE_AUDIO) < 0) {
        fprintf(stderr, "Couldn't find audio stream in file \"%s\"\n",
                filename);
        exit(1);
    }

    ai.codec = ai.codec_context->codec;
    ai.bytes_per_sample = av_get_bytes_per_sample(ai.codec_context->sample_fmt);
    if (ai.bytes_per_sample == 0)
        ai.bytes_per_sample = 4;

    _log("Opened codec: name=%s", ai.codec_context->codec->name);
    _log("Bytes per sample = %d", ai.bytes_per_sample);

    av_dump_format(ai.format_context, 0, ai.filename, 0);

    return ai;
}

int get_sample_rate(audio_info ai) {
    return ai.codec_context->sample_rate;
}

static int64_t print_frame(const AVFrame* frame, int channel_index,
                           int bytes_per_sample) {
    _log("FRAME START, nb_samples=%d", frame->nb_samples);
    const int8_t* p =
        (int8_t*)frame->data[0] + channel_index * bytes_per_sample;
    for (int i = 0; i < frame->nb_samples;
         ++i, p += frame->ch_layout.nb_channels * bytes_per_sample) {
        switch (bytes_per_sample) {
        case 2:
            printf("%d\n", *(int16_t*)p);
            break;
        case 4:
            printf("%f\n", *(float*)p);
            break;
        case 8:
            printf("%lf\n", *(double*)p);
            break;
        default:
            printf("unsupported bytes per sample\n");
            break;
        }
    }
    _log("FRAME END, nb_samples=%d", frame->nb_samples);
    return frame->nb_samples;
}

static size_t decode_packet(AVPacket* packet, AVFrame* frame, audio_info ai) {
    AVCodecContext* codec_context = ai.codec_context;
    int response;
    if ((response = avcodec_send_packet(codec_context, packet)) < 0) {
        fprintf(stderr, "Error while sending a packet to the decoder: %s\n",
                av_err2str(response));
        exit(response);
    }

    size_t total_samples = 0;
    while (response >= 0) {
        response = avcodec_receive_frame(codec_context, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            fprintf(stderr,
                    "Error while receiving a frame from the decoder: %s\n",
                    av_err2str(response));
            exit(response);
        }
        total_samples += frame->nb_samples;
        print_frame(frame, ai.channel_index, ai.bytes_per_sample);
    }
    return total_samples;
}

stream_data extract_audio(audio_info ai) {
    stream_data sd;

    sd.data = NULL;
    sd.size = 0;

    AVFrame* frame;
    if ((frame = av_frame_alloc()) == NULL) {
        fprintf(stderr, "Could not allocate frame\n");
        exit(1);
    }
    AVPacket* packet;
    if ((packet = av_packet_alloc()) == NULL) {
        fprintf(stderr, "Could not allocate packet\n");
        exit(1);
    }

    size_t total_samples = 0;
    while (av_read_frame(ai.format_context, packet) >= 0) {
        if (packet->stream_index == ai.stream_index) {
            int samples_in_packet = decode_packet(packet, frame, ai);
            total_samples += samples_in_packet;
        }
        av_packet_unref(packet);
    }
    _log("Total samples = %ld", total_samples);

    av_frame_free(&frame);
    av_packet_free(&packet);
    return sd;
}

int get_delta_samples(stream_data sd1, stream_data sd2) {
    size_t size = sd1.size + sd2.size - 1;

    _log("Required memory for fft: %ld\n",
         sizeof(double) * size + sizeof(fftw_complex) * 2 * (size / 2 + 1));
    double* in = (double*)fftw_malloc(sizeof(double) * size);
    fftw_complex* out1 =
        (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (size / 2 + 1));
    fftw_complex* out2 =
        (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (size / 2 + 1));

    if (in == NULL || out1 == NULL || out2 == NULL) {
        fprintf(stderr, "Could not allocate memory for fft\n");
        fftw_free(in);
        fftw_free(out1);
        fftw_free(out2);
        free_stream_data(sd1);
        free_stream_data(sd2);
        exit(1);
    }

    // fft for sd1
    fftw_plan p1 = fftw_plan_dft_r2c_1d(size, in, out1, FFTW_ESTIMATE);
    for (size_t i = 0; i < size; ++i) {
        if (i < sd1.size) {
            in[i] = (double)sd1.data[i];
        } else {
            in[i] = 0;
        }
    }
    fftw_execute(p1);
    fftw_destroy_plan(p1);

    // fft for sd2
    fftw_plan p2 = fftw_plan_dft_r2c_1d(size, in, out2, FFTW_ESTIMATE);
    for (size_t i = 0; i < size; ++i) {
        if (i < sd2.size) {
            in[i] = (double)sd2.data[i];
        } else {
            in[i] = 0;
        }
    }
    fftw_execute(p2);
    fftw_destroy_plan(p2);

    // inverse fft for out1 * conj(out2)
    fftw_plan p3 = fftw_plan_dft_c2r_1d(size, out1, in, FFTW_ESTIMATE);
    for (size_t i = 0; i < (size / 2 + 1); ++i) {
        out1[i] *= conj(out2[i]);
    }
    fftw_execute(p3);

    // find maximum
    size_t delta = 0;
    double peak = in[0];
    for (size_t i = 0; i < size; ++i) {
        if (peak < in[i]) {
            peak = in[i];
            delta = i;
        }
    }

    // make negative if bigger than array size
    if (delta >= sd1.size) {
        delta -= size;
    }
    return delta;
}

void free_stream_data(stream_data sd) {
    free(sd.data);
}

void free_audio_info(audio_info ai) {
    avformat_free_context(ai.format_context);
    avcodec_free_context(&ai.codec_context);
}
