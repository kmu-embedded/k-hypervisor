include ${SOURCE_PATH}/lib/c/src/arch-arm/Makefile
include ${SOURCE_PATH}/lib/c/src/sys-baremetal/Makefile

objs-y :=

#FOR ALL STANDARD C LIBRARY
#LIBC_SRCS+= memcpy.c strncat.c fgetc.c strpbrk.c fflush.c strcat.c rand.c memcmp.c  \
			clearerr.c strcspn.c calloc.c vprintf.c strrchr.c putchar.c srand.c sprintf.c \
			feof.c system.c realloc.c memset.c ftell.c clock.c time.c strerror.c vsnprintf.c \
			strstr.c fwrite.c gmtime.c fputc.c ferror.c format.c strcmp.c strlen.c difftime.c \
			locale.c memmove.c strcoll.c errno.c strncmp.c strftime.c fputs.c malloc.c  \
			printf.c strtoul.c remove.c getenv.c getchar.c snprintf.c vfprintf.c fclose.c \
			fscanf.c assert.c puts.c strchr.c asctime.c fread.c strtod.c tmpfile.c ungetc.c \
			memchr.c localtime.c fseek.c strspn.c rename.c exit.c strncpy.c mktime.c strtol.c \
			qsort.c rewind.c strtok.c init_libc.c strcpy.c aligned_alloc.c fprintf.c fgets.c \
			strdup.c ctype.c

SUBDIRECTORIES += lib/c/src/arch-arm
SUBDIRECTORIES += lib/c/src/sys-baremetal
SUBDIRECTORIES += lib/c/src/sys-baremetal/arch-arm

objs-y += init_libc.o malloc.o strlen.o strcpy.o printf.o memset.o realloc.o
objs-y += fputc.o memcpy.o vfprintf.o fprintf.o format.o assert.o
objs-y += sprintf.o

obj-y += $(patsubst %, lib/c/src/%, ${objs-y})
