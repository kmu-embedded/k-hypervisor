#include <stdio.h>
#include <l4/kdebug.h>

static size_t
l4kdb_write(const void *data, long int position, size_t count, void *handle /*unused*/)
{
	size_t i;
	const char *realdata = data;
	for (i = 0; i < count; i++)
		L4_KDB_PrintChar(realdata[i]);
	return count;
}

static size_t
l4kdb_read(void *data, long int position, size_t count, void *handle /*unused*/)
{
	size_t i;
	char *realdata = data;
	for (i = 0; i < count; i++)
		realdata[i] = L4_KDB_ReadChar_Blocked();
	return count;
}


struct __file __stdin = {
	NULL,
	l4kdb_read,
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
	l4kdb_write,
	NULL,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};


struct __file __stderr = {
	NULL,
	NULL,
	l4kdb_write,
	NULL,
	NULL,
	_IONBF,
	NULL,
	0,
	0
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
