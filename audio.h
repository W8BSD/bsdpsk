#ifndef AUDIO_H
#define AUDIO_H

#include <unistd.h>	// ssize_t

struct audio;

struct audio *setup_audio_in(const char *dsp_name, int dsp_channels, int dsp_rate);
ssize_t audio_read(struct audio *audio, void *buf, size_t bytes);

#endif
