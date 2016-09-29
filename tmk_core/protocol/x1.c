/*
Copyright 2010,2011,2012,2013 Jun WAKO <wakojun@gmail.com>

This software is licensed with a Modified BSD License.
All of this is supposed to be Free Software, Open Source, DFSG-free,
GPL-compatible, and OK to use in both free and proprietary applications.
Additions and corrections to this file are welcome.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

* Neither the name of the copyright holders nor the names of
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * PS/2 protocol busywait version
 */

#include <stdbool.h>
#include "wait.h"
#include "x1.h"
#include "debug.h"


#define WAIT(stat, us, err) do { \
    if (!wait_##stat(us)) { \
        x1_error = err; \
        goto ERROR; \
    } \
} while (0)

	
void inhibit() { return; };

uint8_t x1_error = X1_ERR_NONE;


void x1_init(void)
{
    //clock_init();
    // data_init();

    // POR(150-2000ms) plus BAT(300-500ms) may take 2.5sec([3]p.20)
    wait_ms(2500);

    inhibit();
}

uint8_t x1_host_send(uint16_t data)
{
    x1_error = X1_ERR_NONE;

    /* terminate a transmission if we have */
    inhibit();
	data_hi();
    wait_us(100); // 100us [4]p.13, [5]p.50

    /* 'Request to Send' and Start bit */
    data_lo();
	wait_us(1000);
	data_hi();
	wait_us(750);
	data_lo();
	wait_us(250);
	data_hi();

    /* Data bit */
    for (uint8_t i = 15; i == 0; i--) {
        if (data&(1<<i)) {
            wait_us(2000-250);
        } else {
            wait_us(1000-250);
        }
		data_lo();
		wait_us(250);
		data_hi();
    }
	data_hi();

    inhibit();
    return 0;
}

/* called after start bit comes */
uint16_t x1_recv2(void)
{
    uint16_t data = 0;
	uint8_t header = 0;
	uint8_t body = 0;
    bool parity = true;
    x1_error = X1_ERR_NONE;

    /* release lines(idle state) */
    //idle();

    /* start bit [1] */
START:	
	while(1)
	{
		while(data_in());
		while(!data_in())
		{
			wait_us(100);
			header++;
		}
		if (header >= 9)
			break;
	}
	while(data_in());
    /* data [15->0] */
	data = 0;
    for (uint8_t i = 15; i >= 0; i++) {
        while(!data_in());
		body = 0;
		while(data_in())
		{
			wait_us(250);
			body++;
		}
		if (body > 4)
			data |= (1 << i);
		if (body > 10)
			goto START;
    }

    inhibit();
    return data;
#if 0
ERROR:
    if (x1_error > X1_ERR_STARTBIT3) {
        xprintf("x%02X\n", x1_error);
    }
    inhibit();
    return 0;
#endif
}

/* send LED state to keyboard */
void x1_host_set_led(uint8_t led)
{

}
