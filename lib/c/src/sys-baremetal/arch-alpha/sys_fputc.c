#include <stdio.h>
#include "sys_console.h"

int
fputc(int c, FILE *stream)
{
	char buf[2];

	buf[0] = c;
	buf[1] = 0;
	console_puts(buf, 1);
	if(c == '\n')
		fputc('\r', stream);

	return 0;
}
