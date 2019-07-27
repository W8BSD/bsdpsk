/*-
 * Copyright (c) 2019 Stephen Hurd, W8BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio.h"

struct audio {
	int	dsp;
};

struct audio *
setup_audio_in(const char *dsp_name, int dsp_channels, int dsp_rate)
{
	struct audio *ret;
	int i;

	ret = calloc(1, sizeof(struct audio));
	if (ret == NULL)
		goto fail;

	/* Open audio device */
	ret->dsp = open(dsp_name, O_RDONLY);
	if (ret->dsp == -1)
		goto fail;

	/* Configure audio device */
	i = AFMT_S16_NE;
	if (ioctl(ret->dsp, SNDCTL_DSP_SETFMT, &i) == -1)
		goto fail;
	if (i != AFMT_S16_NE)
		goto fail;
	if (ioctl(ret->dsp, SNDCTL_DSP_CHANNELS, &dsp_channels) == -1)
		goto fail;
	if (ioctl(ret->dsp, SNDCTL_DSP_SPEED, &dsp_rate) == -1)
		goto fail;

	return (ret);

fail:
	free(ret);
	return (NULL);
}

ssize_t
audio_read(struct audio *audio, void *buf, size_t bytes)
{
	if (audio == NULL || buf == NULL)
		return -1;

	return (read(audio->dsp, buf, bytes));
}
