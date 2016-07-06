#ifndef __SMP_H__
#define __SMP_H__

#include <stdint.h>
#include "sysctrl.h"
#include "cp15.h"
#include "barrier.h"
#include "local_irq.h"

/**
 * @brief Gets current CPU ID of the Symmetric MultiProcessing(SMP).
 *
 * Read the value from Multiprocessor ID Register(MPIDR) and obtains the CPU ID
 * by masking.
 * - Coretex-A15
 *   - MPIDR[1:0] - CPUID - 0, 1, 2, OR 3
 *   - MPIDR[7:2] - Reserved, Read as zero
 * @return The current CPU ID.
 */
#define NR_MAX_CLUSTERS			15
#define CLUSTER_BIT_MASK		(NR_MAX_CLUSTERS << 8)
#define NR_CPUS_PER_CLUSTER     4
#define NR_CPUS_BIT_MASK		0x3

static inline uint32_t smp_processor_id(void)
{
    uint32_t mpidr = read_cp32(MPIDR);
    uint8_t cluster_id = ((mpidr & CLUSTER_BIT_MASK) >> 8) * NR_CPUS_PER_CLUSTER;

    return (mpidr & NR_CPUS_BIT_MASK) + cluster_id;
}

#endif
