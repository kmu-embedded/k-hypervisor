/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 National ICT Australia
 * 
 * All rights reserved. 
 * 
 * Developed by: Embedded, Real-time and Operating Systems Program (ERTOS)
 *               National ICT Australia
 *               http://www.ertos.nicta.com.au
 * 
 * Permission is granted by National ICT Australia, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of National ICT Australia, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on National ICT
 * Australia or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of National ICT Australia or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */

#ifndef _SYS_CONSOLE_H_
#define _SYS_CONSOLE_H_

#include <stdlib.h>

#define BUFSIZE 256

#define INIT_HWRPB (void*) 0x10000000

/*
 * Console callback routine numbers
 */
#define CCB_GETC		0x01
#define CCB_PUTS		0x02
#define CCB_RESET_TERM		0x03
#define CCB_SET_TERM_INT	0x04
#define CCB_SET_TERM_CTL	0x05
#define CCB_PROCESS_KEYCODE	0x06

#define CCB_OPEN		0x10
#define CCB_CLOSE		0x11
#define CCB_IOCTL		0x12
#define CCB_READ		0x13
#define CCB_WRITE		0x14

#define CCB_SET_ENV		0x20
#define CCB_RESET_ENV		0x21
#define CCB_GET_ENV		0x22
#define CCB_SAVE_ENV		0x23

#define CCB_PSWITCH		0x30
#define CCB_BIOS_EMUL		0x32
/*
 * Environment variable numbers
 */
#define ENV_AUTO_ACTION		0x01
#define ENV_BOOT_DEV		0x02
#define ENV_BOOTDEF_DEV		0x03
#define ENV_BOOTED_DEV		0x04
#define ENV_BOOT_FILE		0x05
#define ENV_BOOTED_FILE		0x06
#define ENV_BOOT_OSFLAGS	0x07
#define ENV_BOOTED_OSFLAGS	0x08
#define ENV_BOOT_RESET		0x09
#define ENV_DUMP_DEV		0x0A
#define ENV_ENABLE_AUDIT	0x0B
#define ENV_LICENSE		0x0C
#define ENV_CHAR_SET		0x0D
#define ENV_LANGUAGE		0x0E
#define ENV_TTY_DEV		0x0F

#define NO_SRM_CONSOLE		-1L

#define CTB_TYPE_NONE     0
#define CTB_TYPE_DETACHED 1
#define CTB_TYPE_SERIAL   2
#define CTB_TYPE_GRAPHICS 3
#define CTB_TYPE_MULTI    4

struct ctb_struct {
	unsigned long type;
	unsigned long id;
	unsigned long reserved;
	unsigned long dsd_len;
	char dsd[0];
};


int64_t console_getenv(uint64_t i, char *env, uint64_t maxlen);
int64_t console_puts(const char *str, uint64_t len);

extern void pal_halt(void);
extern int64_t console_dispatch(uint64_t a0, uint64_t a1,
				uint64_t a2, uint64_t a3,
				void *hwrpb);


#endif /* !_SYS_CONSOLE_H_ */
