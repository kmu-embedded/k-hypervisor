#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE * sys_tmpfile(void);

struct tmp {
	long int total_size;
	char * buffer;
};

static long int
tmp_eof(void *handle)
{
	struct tmp *tmp = handle;
	return tmp->total_size;
}

static size_t
tmp_write(const void *data, long int position, size_t count, void *handle)
{
	void *tmp_buf;
	struct tmp *tmp = handle;
	if (position + count > tmp->total_size) {
		tmp->total_size = position + count;
		tmp_buf = realloc(tmp->buffer, tmp->total_size);
		assert(tmp_buf != NULL);
		tmp->buffer = tmp_buf;
	}
	memcpy(&tmp->buffer[position], data, count);
	return count;
}

static size_t
tmp_read(void *data, long int position, size_t count, void *handle)
{
	struct tmp *tmp = handle;
	if (position + count > tmp->total_size) {
	    count = tmp->total_size - position;
	}
	memcpy(data, &tmp->buffer[position], count);
	return count;
}

static int
tmp_close(void *handle)
{
	struct tmp *tmp = handle;
	free(tmp->buffer);
	free(handle);
	return 0;
}

FILE *
sys_tmpfile(void)
{
	/* Note: We may want to allocate from a different pool
	   of memory to minimise this being tramped on a by a
	   user */
	struct tmp *tmp;
	FILE *tmpf;
	tmpf = malloc(sizeof(FILE));
	if (tmpf == NULL)
		return NULL;
	tmp = malloc(sizeof(struct tmp));
	if (tmp == NULL) {
		free(tmpf);
		return NULL;
	}

	tmp->total_size = 0;
	tmp->buffer = NULL;

	tmpf->handle = tmp;
	tmpf->write_fn = tmp_write;
	tmpf->read_fn = tmp_read;
	tmpf->close_fn = tmp_close;
	tmpf->eof_fn = tmp_eof;
	tmpf->current_pos = 0;
	tmpf->buffering_mode = _IONBF;
	tmpf->buffer = NULL;
	tmpf->unget_pos = 0;
	tmpf->eof = 0;
#ifdef THREAD_SAFE
	mutex_init(&tmpf->mutex);
#endif

	return tmpf;
}
