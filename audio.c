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
