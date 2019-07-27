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

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "filters.h"

double
fir_filter(int16_t value, struct fir_filter *f)
{
	size_t i;
	float res = 0;

	memmove(f->buf, &f->buf[1], sizeof(f->buf[0]) * (f->len - 1));
	f->buf[f->len - 1] = (float)value;

#pragma clang loop vectorize(enable)
	for (i = 0; i < f->len; i++)
		res += f->buf[i] * f->coef[i];

	return (res / f->len);
}

struct fir_filter *
create_matched_filter(double frequency, int dsp_rate, int len)
{
	size_t i;
	struct fir_filter *ret;
	double wavelen;

	ret = malloc(sizeof(*ret));
	if (ret == NULL)
		return (NULL);
	/*
	 * For the given sample rate, calculate the number of
	 * samples in a complete wave
	 */
	ret->len = len;
	wavelen = dsp_rate / frequency;

	ret->buf = calloc(sizeof(*ret->buf), ret->len);
	if (ret->buf == NULL) {
		free(ret);
		return (NULL);
	}
	ret->coef = malloc(sizeof(*ret->coef) * ret->len);
	if (ret->coef == NULL) {
		free(ret->buf);
		free(ret);
		return (NULL);
	}

	/*
	 * Now create a sine wave with that many samples in coef
	 */
	for (i = 0; i < ret->len; i++)
		ret->coef[ret->len - i - 1] = sin((double)i / wavelen * (2.0 * M_PI));

	return (ret);
}

double
inverted_fir_filter(struct fir_filter *f)
{
	size_t i;
	float res = 0;

#pragma clang loop vectorize(enable)
	for (i = 0; i < f->len; i++)
		res -= f->buf[i] * f->coef[i];

	return (res / f->len);
}

void
free_fir_filter(struct fir_filter *f)
{
	if (f) {
		if (f->buf)
			free(f->buf);
		if (f->coef)
			free(f->coef);
		free(f);
	}
}
