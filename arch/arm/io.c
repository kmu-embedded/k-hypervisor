#include <io.h>

void write64(uint64_t value, addr_t addr)
{
    uint32_t upper = 0, lower = 0;
    upper = (value >> 32) & 0xFFFFFFFF;
    lower = (value & 0x00000000FFFFFFFFUL);
    __writel(lower, addr);
    __writel(upper, addr + 0x4);
}

uint64_t read64(addr_t addr)
{
    uint64_t result = 0x0UL;
    result |= __readl(addr);
    result |= (uint64_t) __readl(addr + 0x4) << 32;
    return result;
}
