#ifndef PSK_DEMOD_H
#define PSK_DEMOD_H

struct psk_rx;

struct psk_rx *setup_rx(double freq, int dsp_rate);
int get_psk_ch(struct psk_rx *rx, struct audio *a);

#endif
