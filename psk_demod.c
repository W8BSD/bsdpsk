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

struct psk_rx {
	struct fir_filter *fir;
	struct fir_filter *matched;
	struct bq_filter *bpf;
	double freq;
	double peak;
	double squelch;
	double symbol_rate;
	int dsp_rate;
	int state;
	int peak_ago;
};

static double
calc_q(double freq, double delta)
{
	return (freq/delta);
}

struct psk_rx *
setup_rx(double symbol_rate, double freq, int dsp_rate, double squelch)
{
	struct psk_rx *ret;

	ret = calloc(1, sizeof(struct psk_rx));
	if (ret == NULL)
		goto fail;

	ret->dsp_rate = dsp_rate;
	ret->freq = freq;
	ret->squelch = squelch;
	ret->symbol_rate = symbol_rate;

	ret->fir = create_matched_filter(ret->freq, ret->dsp_rate, ((double)ret->dsp_rate)/ret->symbol_rate);
	if (ret->fir == NULL)
		goto fail;
	ret->matched = create_matched_filter(ret->freq, ret->dsp_rate, ((double)ret->dsp_rate)/ret->symbol_rate);
	if (ret->fir == NULL)
		goto fail;
	ret->bpf = calc_bpf_coef(ret->freq, calc_q(freq, ret->symbol_rate*2), ret->dsp_rate);
	if (ret->bpf == NULL)
		goto fail;

	return ret;
fail:
	if (ret != NULL) {
		if (ret->bpf != NULL)
			free_bq_filter(ret->bpf);
		if (ret->fir != NULL)
			free_fir_filter(ret->fir);
		if (ret->matched != NULL)
			free_fir_filter(ret->matched);
		free(ret);
	}
	return (NULL);
}

static int
get_psk_bit(struct psk_rx *rx, struct audio *a)
{
	struct fir_filter *new;
	double bp, d, ad;
	int16_t buf;
	int c = 0;
	int sc = rx->dsp_rate / rx->symbol_rate;
	int hsc = sc / 2;
	bool ret;

	switch(rx->state) {
		case 0:
			// TODO: The time this takes is unbounded...
			rx->peak_ago = 0;
			rx->peak = 0.0;
			while(audio_read(a, &buf, sizeof(buf)) == sizeof(buf)) {
				bp = bq_filter(buf, rx->bpf);
				d = fir_filter(bp, rx->matched);
				ad = fabs(d);
				rx->peak_ago++;
				if (ad > rx->peak) {
					rx->peak = ad;
					rx->peak_ago = 0;
				}
				if (rx->peak_ago == hsc && rx->peak > rx->squelch) {
					rx->state = 1;
					break;
				}
			}
			// Advance to the next peak...
			memmove(&rx->matched->buf[0], &rx->matched->buf[sc / 2], rx->matched->len - (sc / 2));
			for (c = 0; c < hsc; c++) {
				audio_read(a, &buf, sizeof(buf));
				rx->matched->buf[sc / 2 + c] = bq_filter(buf, rx->bpf);
			}
			d = fir_filter_calc(rx->matched);
			if (fabs(d) > rx->squelch) {
				rx->state = 1;
				memcpy(rx->fir->buf, rx->matched->buf, rx->fir->len * sizeof(*rx->fir->buf));
				return (0);
			}
			// False alarm...
			return (-1);
		case 1:
			// We're at a "peak"...
			// TODO: Clock recovery
			memcpy(rx->fir->coef, rx->fir->buf, rx->fir->len * sizeof(*rx->fir->coef));
			for (c=0; c<sc; c++) {
				if(audio_read(a, &buf, sizeof(buf)) == sizeof(buf))
					rx->fir->buf[c] = bq_filter(buf, rx->bpf);
				else
					return (-1);
			}
			d = fir_filter_calc(rx->fir);
			if (fabs(d) < rx->squelch) {
				memcpy(rx->matched->buf, rx->fir->buf, rx->matched->len * sizeof(*rx->matched->buf));
				return -1;
			}
			if (d < 0.0)
				ret = false;
			else
				ret = true;
			return (ret);
		default:
			break;
	}
	return (-1);
}

// TODO: Maybe this should be passed in samples instead of reading itself.
int
get_psk_ch(struct psk_rx *rx, struct audio *a)
{
	uint16_t ch = 0;
	int bit;

	while (1) {
		ch <<= 1;
		bit = get_psk_bit(rx, a);
		if (bit == -1) {
			rx->state = 0;
			return 0;
		}
		ch |= bit;
		if (ch) {
			if ((ch & 0x03) == 0) {
				return varicode2asc(ch>>2);
			}
		}
	}
}
