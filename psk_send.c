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

#include <sys/types.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "filters.h"
#include "varicode.h"

struct psk_tx {
	double *envelope;
	double freq;
	double symbol_rate;
	size_t envelope_len;
	size_t wave_len;
	int dsp_rate;
	bool inverted;
	int16_t *wave;
};

struct psk_tx *
setup_tx(double symbol_rate, double freq, int dsp_rate)
{
	struct psk_tx *ret;
	double wavelen = dsp_rate / freq;
	int i;

	ret = calloc(1, sizeof(struct psk_tx));
	if (ret == NULL)
		goto fail;

	ret->dsp_rate = dsp_rate;
	ret->freq = freq;
	ret->symbol_rate = symbol_rate;
	ret->envelope_len = dsp_rate / symbol_rate / 2;
	ret->envelope = malloc(sizeof(*ret->envelope) * ret->envelope_len);
	if (ret->envelope == NULL)
		goto fail;
	ret->wave_len = dsp_rate / symbol_rate;
	ret->wave = malloc(sizeof(*ret->wave) * ret->wave_len);
	if (ret->wave == NULL)
		goto fail;

	for (i = 0; i < ret->envelope_len; i++)
		ret->envelope[i] = cos((double)i / ret->envelope_len * (M_PI / 2));

	for (i = 0; i < ret->wave_len; i++)
		ret->wave[i] = sin((double)i / wavelen * (2.0 * M_PI)) * INT16_MAX;

	return ret;
fail:
	if (ret != NULL) {
		if (ret->envelope != NULL)
			free(ret->envelope);
		if (ret->wave != NULL)
			free(ret->wave);
		free(ret);
	}
	return (NULL);
}

static int
send_psk_bit(struct psk_tx *tx, struct audio *a, bool bit)
{
	int i, j;
	int16_t buf;

	if (bit) {
		// Constant carrier, no problem
		for (i = 0; i < tx->wave_len; i++) {
			buf = tx->wave[i];
			if (tx->inverted)
				buf = 0 - buf;
			if (audio_write(a, &buf, sizeof(buf)) != sizeof(buf))
				return -1;
		}
	}
	else {
		// First half of the wave, taper down
		for (i = 0; i < tx->envelope_len; i++) {
			buf = tx->wave[i] * tx->envelope[i];
			if (tx->inverted)
				buf = 0 - buf;
			if (audio_write(a, &buf, sizeof(buf)) != sizeof(buf))
				return -1;
		}
		tx->inverted = !tx->inverted;
		// Second half, taper up.
		for (j = tx->envelope_len - 1; i < tx->wave_len; i++, j--) {
			if (j < 0)
				j = 0;
			buf = tx->wave[i] * tx->envelope[j];
			if (tx->inverted)
				buf = 0 - buf;
			if (audio_write(a, &buf, sizeof(buf)) != sizeof(buf))
				return -1;
		}
	}
	return 0;
}

// TODO: Maybe this should be passed in samples instead of reading itself.
int
send_psk_ch(struct psk_tx *tx, struct audio *a, char ch)
{
	uint16_t c = asc2varicode(ch);

	if (c == 0)
		return 0;
	while ((c & 0x8000) == 0)
		c <<= 1;
	while (c) {
		if (send_psk_bit(tx, a, c >> 15) == -1)
			return -1;
		c <<= 1;
	}
	if (send_psk_bit(tx, a, false) == -1)
		return -1;
	if (send_psk_bit(tx, a, false) == -1)
		return -1;

	return 0;
}

static int
half_bit_start(struct psk_tx *tx, struct audio *a)
{
	int i, j;
	int16_t buf;

	for (j = tx->envelope_len - 1, i = tx->envelope_len; i < tx->wave_len; i++, j--) {
		if (j < 0)
			j = 0;
		buf = tx->wave[i] * tx->envelope[j];
		if (tx->inverted)
			buf = 0 - buf;
		if (audio_write(a, &buf, sizeof(buf)) != sizeof(buf))
			return -1;
	}
	return 0;
}

int
send_psk_start(struct psk_tx *tx, struct audio *a)
{
	int i;

	if (half_bit_start(tx, a) == -1)
		return -1;
	for (i = 0; i < 32; i++) {
		if (send_psk_bit(tx, a, false) == -1)
			return -1;
	}
	return 0;
}

static int
half_bit_end(struct psk_tx *tx, struct audio *a)
{
	int i;
	int16_t buf;

	for (i = 0; i < tx->envelope_len; i++) {
		buf = tx->wave[i] * tx->envelope[i];
		if (tx->inverted)
			buf = 0 - buf;
		if (audio_write(a, &buf, sizeof(buf)) != sizeof(buf))
			return -1;
	}
	return 0;
}

int
send_psk_end(struct psk_tx *tx, struct audio *a)
{
	int i;

	if (half_bit_end(tx, a) == -1)
		return -1;
	for (i = 0; i < 32; i++) {
		if (send_psk_bit(tx, a, true) == -1)
			return -1;
	}
	return 0;
}
