#ifndef FILTERS_H
#define FILTERS_H

#include <inttypes.h>

struct fir_filter {
	size_t		len;
	float		*buf;
	float		*coef;
};

struct bq_filter {
	double		coef[5];
	double		buf[4];
};

struct bq_filter *calc_lpf_coef(double f0, double q, int dsp_rate);
struct bq_filter *calc_bpf_coef(double f0, double q, int dsp_rate);
struct bq_filter *calc_apf_coef(double f0, double q, int dsp_rate);
double bq_filter(double value, struct bq_filter *f);
double fir_filter(int16_t value, struct fir_filter *f);
struct fir_filter *create_matched_filter(double frequency, int dsp_rate, int len);
double inverted_fir_filter(struct fir_filter *f);
void free_bq_filter(struct bq_filter *f);
void free_fir_filter(struct fir_filter *f);

#endif
