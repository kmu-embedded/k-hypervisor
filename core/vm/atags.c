#include <atags.h>
#include <size.h>
#include <string.h>
#include <stdio.h>

// TODO(casionwoo): Refactoring whole code in atags.c

static struct atag *_params = INVALID; /* used to point at the current tag */
static void setup_core_tag(void *address, long pagesize)
{
    /* Initialise parameters to start at given address */
    _params = (struct atag *)address;
    /* start with the core tag */
    _params->hdr.tag = ATAG_CORE;
    _params->hdr.size = tag_size(atag_core);  /* size the tag */
    _params->u.core.flags = 1;                /* ensure read-only */
    _params->u.core.pagesize = pagesize;      /* systems pagesize (4k) */
    /* zero root device (typicaly overidden from commandline )*/
    _params->u.core.rootdev = 0;
    /* move pointer to next tag */
    _params = tag_next(_params);              /* move pointer to next tag */
}

static void setup_revision_tag(void)
{
    _params->hdr.tag = ATAG_REVISION;
    _params->hdr.size = tag_size(atag_revision);
    _params->u.revision.rev = 0xcfdfdfdf;
    _params = tag_next(_params);
}

static void setup_cmdline_tag(const char *line)
{
    int linelen = strlen(line);
    /* do not insert a tag for an empty commandline */
    if (!linelen) {
        return;
    }
    _params->hdr.tag = ATAG_CMDLINE;          /* Commandline tag */
    _params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;
    /* place commandline into tag */
    strcpy(_params->u.cmdline.cmdline, line);
    _params = tag_next(_params);              /* move pointer to next tag */
}

static void setup_mem_tag(uint32_t start, uint32_t len)
{
    _params->hdr.tag = ATAG_MEM;             /* Memory tag */
    _params->hdr.size = tag_size(atag_mem);  /* size tag */
    /* Start of memory area (physical address) */
    _params->u.mem.start = start;
    _params->u.mem.size = len;               /* Length of area */
    _params = tag_next(_params);             /* move pointer to next tag */
}

#if 0
static void setup_initrd2_tag(uint32_t start, uint32_t size)
{
    _params->hdr.tag = ATAG_INITRD2;        /* Initrd2 tag */
    _params->hdr.size = tag_size(atag_initrd2); /* size tag */
    _params->u.initrd2.start = start;        /* physical start */
    _params->u.initrd2.size = size;          /* compressed ramdisk size */
    _params = tag_next(_params);              /* move pointer to next tag */
}
#endif

static void setup_end_tag(void)
{
    _params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    _params->hdr.size = 0;                   /* zero length */
}

#if 0
static void
setup_ramdisk_tag(uint32_t size)
{
    _params->hdr.tag = ATAG_RAMDISK;         /* Ramdisk tag */
    _params->hdr.size = tag_size(atag_ramdisk);  /* size tag */

    _params->u.ramdisk.flags = 0;            /* Load the ramdisk */
    _params->u.ramdisk.size = size;          /* Decompressed ramdisk size */
    _params->u.ramdisk.start = 0;            /* Unused */

    _params = tag_next(_params);              /* move pointer to next tag */
}
#endif

void linuxloader_setup_atags(uint32_t src)
{
    char *commandline =
        /* mmc-rtsm */
    //    "root=/dev/mmcblk0 rw ip=dhcp "
    //    "rw ip=dhcp earlyprintk console=ttyAMA0 mem=256M";

    /* android-rtsm */
    /*
     * "console=tty0 console=ttyAMA0,38400n8 rootwait ro init=/init "
     * "androidboot.console=ttyAMA0 mem=768M";
     */
    /* nfs-rtsm */
    /*
     * "root=/dev/nfs nfsroot=192.168.0.4:/srv/nfs_simpleroot/ "
     * "rw ip=dhcp earlyprintk console=ttyAMA0 mem=256M";
     */
    "earlyprintk console=ttyAMA0 mem=128M root=/dev/nfs nfsroot=192.168.0.8:/srv/nfsroot/ rw ip=dhcp";
    /* ramdisk-rtsm */
    /*
     "root=/dev/ram rw earlyprintk console=ttyAMA0 "
     "mem=512M rdinit=/sbin/init";
     */
    /* android-arndale board */
    /*
     * "root=/dev/ram0 rw ramdisk=8192 initrd=0x41000000,8M "
     * console=ttySAC1,115200 init= mem=256M"
     */
    /* Arndale board ramdisk */
    // "root=/dev/ram rw earlyprintk console=ttySAC1 "
    // "mem=512M rdinit=/sbin/init";
    /* Arndale board with mmc */
    /*
      "root=/dev/mmcblk1p1   rw ip=dhcp earlyprintk rootwait "
      "console=ttySAC1,115200n8 mem=512M init --no-log";
    */
    /* standard core tag 4k pagesize */
    setup_core_tag((uint32_t *)src, SZ_4K);
    /* commandline setting root device */
    setup_revision_tag();
    setup_mem_tag(src, SZ_512M);
#ifdef USE_ANDROID_INITRD
    {
        uint32_t start = &initrd_start;
        uint32_t end = &initrd_end;
        uint32_t size = end - start;
        setup_initrd2_tag(start + 0x40, size - 0x40);
    }
#endif
    setup_cmdline_tag(commandline);
    /* end of tags */
    setup_end_tag();
}

uint32_t *linuxloader_get_atags_addr(void)
{
    if (_params != INVALID) {
        return (uint32_t *) _params;
    } else {
        printf("[loadlinux] ERROR: SET ATAGS BEFORE JUMP TO ZIMAGE\n");
        while (1)
            ;
    }
}

void atags_setup(void)
{
    int i;

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        linuxloader_setup_atags(CFG_MEMMAP_GUEST0_ATAGS_OFFSET + (i * CFG_MEMMAP_GUEST_SIZE));
    }
}

