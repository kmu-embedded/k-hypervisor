#include <stdio.h>
#include <vdev.h>
#include <drivers/pl180.h>
#include <io.h>

int32_t vmci_read(void *pdata, uint32_t offset);
int32_t vmci_write(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vmci_create(void **pdata);

struct pl180 {
	uint32_t mci_power;
	uint32_t mci_clock;
	uint32_t mci_arg;
	uint32_t mci_cmd;
	uint32_t mci_res_cmd;

	uint32_t mci_res0;
	uint32_t mci_res1;
	uint32_t mci_res2;
	uint32_t mci_res3;

	uint32_t mci_dtimer;
	uint32_t mci_dlength;
	uint32_t mci_dctrl;
	uint32_t mci_dcnt;

	uint32_t mci_status;
	uint32_t mci_clear;

	uint32_t mci_mask0;
	uint32_t mci_mask1;
	uint32_t mci_select;

	uint32_t mci_fifo_cnt;
	uint32_t mci_fifo;

	uint32_t mci_periphid0;
	uint32_t mci_periphid1;
	uint32_t mci_periphid2;
	uint32_t mci_periphid3;

	uint32_t mci_pcellid0;
	uint32_t mci_pcellid1;
	uint32_t mci_pcellid2;
	uint32_t mci_pcellid3;
};

struct vdev_module pl180_vmci = {
		.name = "vdev_pl180",
		.base = PL180_BASE,
		.size = 0x1000,
		.read = vmci_read,
		.write = vmci_write,
		.create = vmci_create,
};

#include <stdlib.h>
int32_t vmci_create(void **pdata) {
	struct pl180 *vmci = malloc(sizeof(struct pl180));

	*pdata = vmci;
	return 0;
}

int32_t vmci_write(void *pdata, uint32_t offset, uint32_t *addr)
{
	struct pl180 *pl180 = pdata;

	switch (offset) {

	case MCI_POWER(0):
		pl180->mci_power = readl(addr);
		writel(pl180->mci_power, MCI_POWER(PL180_BASE));
		break;

	case MCI_CLOCK(0):
		pl180->mci_clock = readl(addr);
		writel(pl180->mci_clock, MCI_CLOCK(PL180_BASE));
		break;

	case MCI_ARG(0):
		pl180->mci_arg = readl(addr);
		writel(pl180->mci_arg, MCI_ARG(PL180_BASE));
		break;

	case MCI_CMD(0):
		pl180->mci_cmd = readl(addr);
		writel(pl180->mci_cmd, MCI_CMD(PL180_BASE));
		break;

	case MCI_RES_CMD(0):
		pl180->mci_res_cmd = readl(addr);
		writel(pl180->mci_res_cmd, MCI_RES_CMD(PL180_BASE));
		break;

	case MCI_DTIMER(0):
		pl180->mci_dtimer = readl(addr);
		writel(pl180->mci_dtimer, MCI_DTIMER(PL180_BASE));
		break;

	case MCI_DLENGTH(0):
		pl180->mci_dlength = readl(addr);
		writel(pl180->mci_dlength, MCI_DLENGTH(PL180_BASE));
		break;

	case MCI_DCTRL(0):
		pl180->mci_dctrl = readl(addr);
		writel(pl180->mci_dctrl, MCI_DCTRL(PL180_BASE));
		break;

	case MCI_DCNT(0):
		pl180->mci_dcnt = readl(addr);
		writel(pl180->mci_dcnt, MCI_DCNT(PL180_BASE));
		break;

	case MCI_CLEAR(0):
		pl180->mci_clear = readl(addr);
		writel(pl180->mci_clear, MCI_CLEAR(PL180_BASE));
		break;

	case MCI_MASK0(0):
		pl180->mci_mask0 = readl(addr);
		writel(pl180->mci_mask0, MCI_MASK0(PL180_BASE));
		break;

	case MCI_MASK1(0):
		pl180->mci_mask1 = readl(addr);
		writel(pl180->mci_mask1, MCI_MASK1(PL180_BASE));
		break;

	case MCI_SELECT(0):
		pl180->mci_select = readl(addr);
		writel(pl180->mci_select, MCI_SELECT(PL180_BASE));
		break;

	case MCI_FIFO(0):
		pl180->mci_fifo = readl(addr);
		writel(pl180->mci_fifo, MCI_FIFO(PL180_BASE));
		break;

	default:
		//printf("%s ERROR!! [write] offset: %x, %x\n", __func__, offset, readl(addr));
		break;
	}
	return 0;
}

int32_t vmci_read(void *pdata, uint32_t offset)
{

	struct pl180 *pl180 = pdata;

	switch (offset) {

	case MCI_RES0(0):
		pl180->mci_res0 = readl(MCI_RES0(PL180_BASE));
		return pl180->mci_res0;
		break;

	case MCI_RES1(0):
		pl180->mci_res1 = readl(MCI_RES1(PL180_BASE));
		return pl180->mci_res1;
		break;

	case MCI_RES2(0):
		pl180->mci_res2 = readl(MCI_RES2(PL180_BASE));
		return pl180->mci_res2;
		break;

	case MCI_RES3(0):
		pl180->mci_res3 = readl(MCI_RES3(PL180_BASE));
		return pl180->mci_res3;
		break;

	case MCI_STATUS(0):
		pl180->mci_status = readl(MCI_STATUS(PL180_BASE));
		return pl180->mci_status;
		break;

	case MCI_FIFO_CNT(0):
		pl180->mci_fifo_cnt = readl(MCI_FIFO_CNT(PL180_BASE));
		return pl180->mci_fifo_cnt;
		break;

	case MCI_PERIPHID0(0):
		pl180->mci_periphid0 = readl(MCI_PERIPHID0(PL180_BASE));
		return pl180->mci_periphid0;
		break;

	case MCI_PERIPHID1(0):
		pl180->mci_periphid1 = readl(MCI_PERIPHID1(PL180_BASE));
		return pl180->mci_periphid1;
		break;

	case MCI_PERIPHID2(0):
		pl180->mci_periphid2 = readl(MCI_PERIPHID2(PL180_BASE));
		return pl180->mci_periphid2;
		break;

	case MCI_PERIPHID3(0):
		pl180->mci_periphid3 = readl(MCI_PERIPHID3(PL180_BASE));
		return pl180->mci_periphid3;
		break;

	case MCI_PCELL0(0):
		pl180->mci_pcellid0 = readl(MCI_PCELL0(PL180_BASE));
		return pl180->mci_pcellid0;
		break;

	case MCI_PCELL1(0):
		pl180->mci_pcellid1 = readl(MCI_PCELL1(PL180_BASE));
		return pl180->mci_pcellid1;
		break;

	case MCI_PCELL2(0):
		pl180->mci_pcellid2 = readl(MCI_PCELL2(PL180_BASE));
		return pl180->mci_pcellid2;
		break;

	case MCI_PCELL3(0):
		pl180->mci_pcellid3 = readl(MCI_PCELL3(PL180_BASE));
		return pl180->mci_pcellid3;
		break;

	default:
		//printf("%s ERROR!! [read] offset: %x, %x\n", __func__, offset);
		break;
	}
	return 0;
}

hvmm_status_t vdev_pl180_init() {
	hvmm_status_t result = HVMM_STATUS_BUSY;

	vdev_register(&pl180_vmci);

	printf("vdev registered:'%s'\n", pl180_vmci.name);

	return result;
}

vdev_module_init(vdev_pl180_init);

