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

#include <stdio.h>

#include "audio.h"
#include "psk_demod.h"
#include "psk_send.h"

int main(int argc, char **argv)
{
	struct audio *audio;
#if 0
	struct psk_rx *psk_rx;

	audio = setup_audio_in("/dev/dsp5", 1, 8000);
	psk_rx = setup_rx(31.25, 1500 /* Freq */, 8000, 100);

	for (;;) {
		printf("%c", get_psk_ch(psk_rx, audio));
		fflush(stdout);
	}
#else
	const char *p;
	const char str[] = "Testing de W8BSD.";
	struct psk_tx *psk_tx;

	audio = setup_audio_out("/dev/dsp4", 1, 8000);
	psk_tx = setup_tx(31.25, 1500 /* Freq */, 8000);

	send_psk_start(psk_tx, audio);
	for (p = str; *p; p++)
		send_psk_ch(psk_tx, audio, *p);
	send_psk_end(psk_tx, audio);
#endif

	return 0;
}
