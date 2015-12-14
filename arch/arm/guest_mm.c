#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_types.h>
#include <hvmm_trace.h>

#include <guest_mm.h>

#include <stage2_mm.h>
#include <stage1_mm.h>

vm_pgentry vm0_l1_pgtable[L1_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm0_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm0_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(4096)));

vm_pgentry vm1_l1_pgtable[L1_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm1_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm1_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(4096)));

static vm_pgentry set_stage2_table(uint32_t paddr);
static vm_pgentry set_stage2_entry(uint32_t paddr, enum memattr mem_attr);

static vm_pgentry set_stage2_table(uint32_t paddr)
{
    vm_pgentry entry;

    entry.raw = 0;
    entry.table.valid = 1;
    entry.table.type = 1;
    entry.table.base = paddr >> PAGE_SHIFT;

    return entry;
}

// TODO(wonseok): configure AP(Access Permission) bit
// Currently, we are going to set AP as a read/write only at PL2.
static vm_pgentry set_stage2_entry(uint32_t paddr, enum memattr mem_attr)
{
    vm_pgentry entry;

    entry.raw = 0;
    entry.page.valid = 1;
    entry.page.type = 1;
    entry.page.base = paddr >> L3_SHIFT;
    entry.page.mem_attr = mem_attr;
    entry.page.ap = 3;
    entry.page.sh = 0;
    entry.page.af = 1;
    entry.page.ng = 0;
    entry.page.cb = 0;
    entry.page.pxn = 0;
    entry.page.xn = 0;

    return entry;
}

// create 4k emtpy mapping
void guest_mm_init()
{
    uint64_t paddr = 0x00000000ULL;
    int i, j, k;

    // set L1 pgtable
    for (i = 0; i < 4; i++) {
        vm0_l1_pgtable[i] = set_stage2_table((uint32_t) vm0_l2_pgtable[i]);
        vm1_l1_pgtable[i] = set_stage2_table((uint32_t) vm1_l2_pgtable[i]);
    }

    // set L2 ptable
    for (i = 0; i < 4; i++) {
        for(j = 0; j < 512; j++) {
            vm0_l2_pgtable[i][j] = set_stage2_table((uint32_t) vm0_l3_pgtable[i][j]);
            vm1_l2_pgtable[i][j] = set_stage2_table((uint32_t) vm1_l3_pgtable[i][j]);
        }
    }

    //set all invalid
    for (i = 0; i < 4; i++) {
        for(j = 0; j < 512; j++) {
            for(k = 0; k < 512; k++) {
                vm0_l3_pgtable[i][j][k].table.valid = 0;
                vm1_l3_pgtable[i][j][k].table.valid = 0;
            }
        }
    }
}

#define TABLE_OFFSET(offs) ((unsigned int)(offs) & 511)
void add_mapping_stage2(uint64_t ipa, uint64_t pa, uint32_t offset, enum memattr mattr, vmid_t vmid)
{
    int first_index;
    int second_index;
    int third_index;
    int i;
    int nr;
    uint64_t base;

    first_index = pa & 0xF0000000;
    first_index = first_index >> 30;

    second_index = pa >> 21;
    second_index = TABLE_OFFSET(second_index);

    third_index = pa >> 12;
    third_index = TABLE_OFFSET(third_index);

    nr = offset / 0x1000;

    for (i = 0; i < nr; i++, pa += 0x1000) {
        if(vmid == 0) {
            vm0_l3_pgtable[first_index][second_index][third_index + i] = set_stage2_entry(pa, mattr);
        } else {
            vm1_l3_pgtable[first_index][second_index][third_index + i] = set_stage2_entry(pa, mattr);
        }
    }
}

#define UART0   0x1C090000
#define GICC    0x2C002000
#define MEM

#define SZ_1M   0x100000
#define SZ_64K  0x10000     // 1 << 16
#define SZ_8K   0x2000

extern uint32_t _guest0_bin_start;
extern uint32_t _guest0_bin_end;
extern uint32_t _guest1_bin_start;
extern uint32_t _guest1_bin_end;

vm_pgentry *_vttbr[4];

void test()
{
    uint64_t pa;

    _vttbr[0] = &vm0_l1_pgtable[0];
    _vttbr[1] = &vm1_l1_pgtable[0];
    guest_mm_init();

    pa = (uint64_t)((uint32_t) &_guest0_bin_start);
    add_mapping_stage2(UART0, 0x1C0A0000, SZ_64K, MEMATTR_DM, 0);
    add_mapping_stage2(GICC, 0x2C002000, SZ_8K, MEMATTR_DM, 0);
    add_mapping_stage2(pa, 0x00000000, (256 * SZ_1M), MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB, 0);

    pa = (uint64_t)((uint32_t) &_guest1_bin_start);
    add_mapping_stage2(UART0, 0x1C0B0000, SZ_64K, MEMATTR_DM, 0);
    add_mapping_stage2(GICC, 0x2C002000, SZ_8K, MEMATTR_DM, 0);
    add_mapping_stage2(pa, 0x00000000, (256 * SZ_1M), MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB, 1);
}
