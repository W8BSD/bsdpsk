#include <stdio.h>

#include "audio.h"
#include "psk_demod.h"

int main(int argc, char **argv)
{
	struct audio *audio;
	struct psk_rx *psk_rx;

	audio = setup_audio_in("/dev/dsp-mixer", 1, 8000);
	psk_rx = setup_rx(1500 /* Freq */, 8000);

	for (;;)
		printf("%c", get_psk_ch(psk_rx, audio));

	return 0;
}
