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

#include "varicode.h"

// Note: A 0 represents a reversal and a 1 represents non-reversal
static const uint16_t varicode[128] = {
	0x02AB,	0x02DB,	0x02ED,	0x0377,	0x02EB,	0x035F,	0x02EF,	0x02FD,
	0x02FF,	0x00EF,	0x001D,	0x036F,	0x02DD,	0x001F,	0x0375,	0x03AB,	
	0x02F7,	0x02F5,	0x03AD,	0x03AF,	0x035B,	0x036B,	0x036D,	0x0357,	
	0x037B,	0x037D,	0x03B7,	0x0355,	0x035D,	0x03BB,	0x02FB,	0x037F,	
	0x0001,	0x01FF,	0x015F,	0x01F5,	0x01DB,	0x02D5,	0x02BB,	0x017F,	
	0x00FB,	0x00F7,	0x016F, 0x01DF,	0x0075,	0x0035,	0x0057,	0x01AF,	
	0x00B7,	0x00BD,	0x00ED,	0x00FF,	0x0177,	0x015B,	0x016B,	0x01AD,	
	0x01AB,	0x01B7,	0x00F5,	0x01BD,	0x01ED,	0x0055,	0x01D7,	0x02AF,	
	0x02BD,	0x007D,	0x00EB,	0x00AD,	0x00B5,	0x0077,	0x00DB,	0x00FD,
	0x0155,	0x007F,	0x01FD,	0x017D,	0x00D7,	0x00BB,	0x00DD,	0x00AB,	
	0x00D5,	0x01DD,	0x00AF,	0x006F,	0x006D,	0x0157,	0x01B5,	0x015D,	
	0x0175,	0x017B,	0x02AD,	0x01F7,	0x01EF,	0x01FB,	0x02BF,	0x016D,	
	0x02DF,	0x000B,	0x005F,	0x002F,	0x002D,	0x0003,	0x003D,	0x005B,	
	0x002B,	0x000D,	0x01EB,	0x00BF,	0x001B,	0x003B,	0x000F,	0x0007,	
	0x003F,	0x01BF,	0x0015,	0x0017,	0x0005,	0x0037,	0x007B,	0x006B,	
	0x00DF,	0x005D,	0x01D5,	0x02B7,	0x01BB,	0x02B5,	0x02D7,	0x03B5
};

uint16_t
asc2varicode(char ch)
{
	int c = ch;

	if (c < 0 || c >= sizeof(varicode) / sizeof(*varicode))
		return 0;
	return varicode[c];
}

int
varicode2asc(uint16_t vc)
{
	int i;

	for (i=0; i < sizeof(varicode) / sizeof(*varicode); i++)
		if (varicode[i] == vc)
			return i;
	return -1;
}
