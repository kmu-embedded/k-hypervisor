#include <stdio.h>
#include <stdint.h>
#include "./sys_console.h"

static size_t
console_write(void *data, long int position, size_t count,
              void *handle /*unused*/)
{
        console_puts(data, count);

	return count;
}

struct __file __stdin = {
	.handle	    = NULL,
	.read_fn    = NULL,
	.write_fn   = NULL,
	.close_fn   = NULL,
	.eof_fn	    = NULL,
	.buffering_mode = _IONBF,
	.buffer	    = NULL,
	.unget_pos  = 0,
	.current_pos = 0,
	.eof	    = 0
};

struct __file __stdout = {
	.handle	    = NULL,
	.read_fn    = NULL,
	.write_fn   = console_write,
	.close_fn   = NULL,
	.eof_fn	    = NULL,
	.buffering_mode = _IONBF,
	.buffer	    = NULL,
	.unget_pos  = 0,
	.current_pos = 0,
	.eof	    = 0
};

struct __file __stderr = {
	.handle	    = NULL,
	.read_fn    = NULL,
	.write_fn   = console_write,
	.close_fn   = NULL,
	.eof_fn	    = NULL,
	.buffering_mode = _IONBF,
	.buffer	    = NULL,
	.unget_pos  = 0,
	.current_pos = 0,
	.eof	    = 0
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
