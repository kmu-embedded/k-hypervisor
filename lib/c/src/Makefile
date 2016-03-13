include ${SOURCE_PATH}/lib/c/src/arch-arm/Makefile
include ${SOURCE_PATH}/lib/c/src/sys-baremetal/Makefile

LIBC_ASMS+= $(patsubst %, arch-arm/%, ${LIBC_ARCH_ASM})
LIBC_SRCS+= $(patsubst %, arch-arm/%, ${LIBC_ARCH_SRCS})

LIBC_SRCS+= $(patsubst %, sys-baremetal/%, ${LIBC_BM_SRCS})

#LIBRARY FOR CURRENT USE, IF YOU WANT TO USE OTHER LIBS, YOU HAVE TO INCLUDE THE FILS AS BELOW
LIBC_SRCS+= init_libc.c malloc.c strlen.c strcpy.c printf.c memset.c realloc.c
LIBC_SRCS+= fputc.c memcpy.c vfprintf.c fprintf.c format.c assert.c

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
