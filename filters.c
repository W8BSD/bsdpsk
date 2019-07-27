#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "filters.h"

struct bq_filter *
calc_lpf_coef(double f0, double q, int dsp_rate)
{
	struct bq_filter *ret;
	double w0, cw0, sw0, a[5], b[5], alpha;

	ret = malloc(sizeof(*ret));
	if (ret == NULL)
		return (NULL);

	w0 = 2.0 * M_PI * (f0 / dsp_rate);
	cw0 = cos(w0);
	sw0 = sin(w0);

	alpha = sw0 / (2.0 * q);
	b[0] = (1.0-cw0)/2.0;
	b[1] = 1.0 - cw0;
	b[2] = (1.0-cw0)/2.0;
	a[0] = 1.0 + alpha;
	a[1] = -2.0 * cw0;
	a[2] = 1.0 - alpha;
	ret->coef[0] = b[0]/a[0];
	ret->coef[1] = b[1]/a[0];
	ret->coef[2] = b[2]/a[0];
	ret->coef[3] = a[1]/a[0];
	ret->coef[4] = a[2]/a[0];
	ret->buf[0] = 0;
	ret->buf[1] = 0;
	ret->buf[2] = 0;
	ret->buf[3] = 0;

	return (ret);
}

struct bq_filter *
calc_bpf_coef(double f0, double q, int dsp_rate)
{
	struct bq_filter *ret;
	double w0, cw0, sw0, a[5], b[5], alpha;

	ret = malloc(sizeof(*ret));
	if (ret == NULL)
		return (NULL);

	w0 = 2.0 * M_PI * (f0 / dsp_rate);
	cw0 = cos(w0);
	sw0 = sin(w0);
	alpha = sw0 / (2.0 * q);

	//b[0] = q * alpha;
	b[0] = alpha;
	b[1] = 0.0;
	b[2] = -(b[0]);
	a[0] = 1.0 + alpha;
	a[1] = -2.0 * cw0;
	a[2] = 1.0 - alpha;
	ret->coef[0] = b[0]/a[0];
	ret->coef[1] = b[1]/a[0];
	ret->coef[2] = b[2]/a[0];
	ret->coef[3] = a[1]/a[0];
	ret->coef[4] = a[2]/a[0];
	ret->buf[0] = 0;
	ret->buf[1] = 0;
	ret->buf[2] = 0;
	ret->buf[3] = 0;

	return ret;
}

struct bq_filter *
calc_apf_coef(double f0, double q, int dsp_rate)
{
	struct bq_filter *ret;
	double w0, cw0, sw0, a[5], b[5], alpha;

	ret = malloc(sizeof(*ret));
	if (ret == NULL)
		return (NULL);

	w0 = 2.0 * M_PI * (f0 / dsp_rate);
	cw0 = cos(w0);
	sw0 = sin(w0);
	alpha = sw0 / (2.0 * q);

	//b[0] = q * alpha;
	b[0] = 1 - alpha;
	b[1] = -2 * cw0;
	b[2] = 1 + alpha;
	a[0] = 1.0 + alpha;
	a[1] = -2.0 * cw0;
	a[2] = 1.0 - alpha;
	ret->coef[0] = b[0]/a[0];
	ret->coef[1] = b[1]/a[0];
	ret->coef[2] = b[2]/a[0];
	ret->coef[3] = a[1]/a[0];
	ret->coef[4] = a[2]/a[0];
	ret->buf[0] = 0;
	ret->buf[1] = 0;
	ret->buf[2] = 0;
	ret->buf[3] = 0;

	return (ret);
}

double
bq_filter(double value, struct bq_filter *f)
{
	double y;

	y = (f->coef[0] * value) +
	    (f->coef[1] * f->buf[0]) + (f->coef[2] * f->buf[1]) -
	    (f->coef[3] * f->buf[2]) - (f->coef[4] * f->buf[3]);
	f->buf[1] = f->buf[0];
	f->buf[0] = value;
	f->buf[3] = f->buf[2];
	f->buf[2] = y;
	return y;
}

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
free_bq_filter(struct bq_filter *f)
{
	if (f)
		free(f);
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
