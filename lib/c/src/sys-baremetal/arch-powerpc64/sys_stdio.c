#include <stdio.h>
#include <stdint.h>

extern void console_putc(const char);

#define COMPORT 0x3f8

static size_t
console_write(char *data, long int position, size_t count, void *handle /*unused*/)
{
	size_t i;
	for (i = 0; i < count; i++)
		console_putc(data[i]);
	return count;
}

struct __file __stdin = {
	NULL,
	NULL,
	NULL,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};


struct __file __stdout = {
	NULL,
	NULL,
	console_write,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};


struct __file __stderr = {
	NULL,
	NULL,
	console_write,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
