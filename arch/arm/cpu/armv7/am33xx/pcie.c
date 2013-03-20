#include <common.h>
#include <malloc.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pcie.h>

#define CMD_STATUS		0x004
#define DBI_CS2			(0x1 << 5)
#define IB_XLT_EN		(0x1 << 2)
#define OB_XLT_EN		(0x1 << 1)

#define OB_SIZE			0x030
#define OB_OFFSET_INDEX(n)	(0x200 + (8 * n))
#define OB_OFFSET_HI(n)		(0x204 + (8 * n))
#define IB_BAR0			0x300
#define IB_START0_LO		0x304
#define IB_START0_HI		0x308
#define IB_OFFSET0		0x30c
#define PCIE_REGS		0x51000000

extern void pcie_pll_config(void);

void pcie_enable_module(void)
{
        debug("\nClear PCIe EP setup.....\n");

        __raw_writel(0x000000FF, 0x48180B10);
        __raw_writel(0, 0x48180578);
        __raw_writel(0, 0x48180510);
        __udelay(50); /*Wait 50 us*/
        __raw_writel(2, 0x48180510);
        __raw_writel(2, 0x48180578);
        __udelay(50); /*Wait 50 us*/
        __raw_writel(0x0000007F, 0x48180B10);
        __udelay(50); /*Wait 50 us*/

        debug("\n PCIe is out reset (PRCM)\n");
}

/* Link training encodings as indicated in DEBUG0 register */
#define LTSSM_STATE_MASK                0x1f
#define LTSSM_STATE_L0                  0x11

void pcie_enable_link(void)
{
	int debug0;

	/* Disable LTSSM */
	writel(readl(0x51000004) & ~1, 0x51000004);

       /* Enable LTSSM */
        __raw_writel(__raw_readl(0x51000004) | 0xb01, 0x51000004);
        debug("LTTSM enabled\n");

        while (1) {
                debug0 = __raw_readl(0x51001728);

                if ((debug0 & LTSSM_STATE_MASK) == LTSSM_STATE_L0)
                        break;
		udelay(500);
#if 0
                debug("\nDebug0 = %#x, Debug1 = #%x",
                                debug0, __raw_readl(0x5100172C));
#endif
        }
        debug("LTTSM trained\n");
}

static void ti_pcie_cfg_inbound_xlat(struct pci_controller *hose,
				     u32 start, u32 end)
{
	writel(DBI_CS2 | readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);
	readl(PCIE_REGS + CMD_STATUS);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_0, 1);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_0, 0x1000 - 1);

	writel(~DBI_CS2 & readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);
	readl(PCIE_REGS + CMD_STATUS);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_0, PCIE_REGS);

	writel(start, PCIE_REGS + IB_START0_LO);
	writel(0, PCIE_REGS + IB_START0_HI);
	writel(1, PCIE_REGS + IB_BAR0);
	writel(start, PCIE_REGS + IB_OFFSET0);

	writel(DBI_CS2 | readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);
	readl(PCIE_REGS + CMD_STATUS);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_1, 1);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_1, end - start);

	writel(~DBI_CS2 & readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);
	readl(PCIE_REGS + CMD_STATUS);

	pci_hose_write_config_dword(hose, PCI_BDF(0,0,0),
				   PCI_BASE_ADDRESS_1,
				   start | PCI_BASE_ADDRESS_MEM_PREFETCH);

	writel(IB_XLT_EN | readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);
}

static void ti_pcie_cfg_outbound_xlat(struct pci_controller *hose,
				      u32 start, u32 end)
{
	int i, xlat_size;

	writel(3 & 0x7, PCIE_REGS + OB_SIZE);

	xlat_size = (1 << (3 & 0x7)) * (1 << 20);

	for (i = 0; (i < 32) && (start < end); i++) {
		writel(start | 1, PCIE_REGS + OB_OFFSET_INDEX(i));
		writel(0, PCIE_REGS + OB_OFFSET_HI(i));
		start += xlat_size;
	}

	writel(OB_XLT_EN | readl(PCIE_REGS + CMD_STATUS),
	       PCIE_REGS + CMD_STATUS);

	pci_hose_write_config_word(hose, PCI_BDF(0,0,0), PCI_COMMAND,
				   PCI_COMMAND_MASTER |
				   PCI_COMMAND_MEMORY);
}

#define TI_PCIE_CFG_SETUP(t,b,d,f)	\
		((t) << 24 | (b) << 16 | (d) << 8 | (f))
#define TI_PCIE_OP_READ(size, type, len)				\
static int								\
ti_pcie_read_config_##size(struct pci_controller *hose,			\
			   pci_dev_t dev, int offset, type *val)	\
{									\
	u32 b, d, f, t = 0;						\
	u32 data = 0;							\
	u32 cfg_space = 0x51001000;					\
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		\
	if (b > 1)							\
		t = 1;							\
	dev = TI_PCIE_CFG_SETUP(t, b, d, f);				\
	if ((b <= 1) && (d != 0))					\
		data = ~0;						\
	else {								\
		if (b > 0)						\
			cfg_space += 0x1000;				\
		writel(dev, 0x51000008);				\
		data = readl(cfg_space + (offset & ~3));		\
	}								\
	if (len == 1)							\
		*val = (data >> (8 * (offset & 3))) & 0xff;		\
	else if (len == 2)						\
		*val = (data >> (8 * (offset & 3))) & 0xffff;		\
	else								\
		*val = data;						\
	return 0;							\
}

#define TI_PCIE_OP_WRITE(size, type, len)				\
static int								\
ti_pcie_write_config_##size(struct pci_controller *hose,		\
			    pci_dev_t dev, int offset, type val)	\
{									\
	u32 b, d, f, t = 0;						\
	u32 cfg_space = 0x51001000;					\
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		\
	if (b > 1)							\
		t = 1;							\
	if (b > 0)							\
		cfg_space += 0x1000;					\
	dev = TI_PCIE_CFG_SETUP(t, b, d, f);				\
	writel(dev, 0x51000008);					\
	if (len == 4)							\
		writel(val, cfg_space + offset);			\
	else if (len == 2)						\
		writew(val, cfg_space + offset);			\
	else								\
		writeb(val, cfg_space + offset);			\
	return 0;							\
}

TI_PCIE_OP_READ(byte, u8, 1)
TI_PCIE_OP_READ(word, u16,2)
TI_PCIE_OP_READ(dword, u32, 4)
TI_PCIE_OP_WRITE(byte, u8, 1)
TI_PCIE_OP_WRITE(word, u16, 2)
TI_PCIE_OP_WRITE(dword, u32, 4)

void ti_pcie_setup_hose(struct pci_controller *hose, int busno)
{
	struct pci_region *r;

	r = hose->regions + hose->region_count;

	/* Outbound MEM */
	pci_set_region(r++,
		       0x20000000,
		       0x20000000,
		       1 << 28,
		       PCI_REGION_MEM);

	/* Outbound I/O */
	pci_set_region(r++,
		       0x40000000,
		       0x40000000,
		       1 << 16,
		       PCI_REGION_IO);

	hose->region_count = r - hose->regions;

	hose->first_busno = busno;

	/* Configuration methods */
	pci_set_ops(hose,
		    ti_pcie_read_config_byte,
		    ti_pcie_read_config_word,
		    ti_pcie_read_config_dword,
		    ti_pcie_write_config_byte,
		    ti_pcie_write_config_word,
		    ti_pcie_write_config_dword);

	pci_register_hose(hose);

	/* Set up RC */
	pci_hose_write_config_word(hose, PCI_BDF(0,0,0), PCI_CLASS_DEVICE,
				   PCI_CLASS_BRIDGE_OTHER);
	ti_pcie_cfg_inbound_xlat(hose, 0x80000000, 0x9fffffff);
	ti_pcie_cfg_outbound_xlat(hose, 0x20000000, 0x2fffffff);

	pciauto_config_init(hose);

	/* Scan bus 0 */
	pci_hose_scan_bus(hose, 0);
	/* Scan bus 1 */
	hose->last_busno = pci_hose_scan_bus(hose, 1);


}

void ti_pcie_init_hoses(void)
{
	struct pci_controller *hose;

	hose = calloc(1, sizeof(struct pci_controller));
	if (!hose) {
		debug("Failed to allocate PCI-E hose\n");
		return;
	}

	/* FIXME gotta pass in an init struct to handle multiple hoses */
	ti_pcie_setup_hose(hose, 0);
}

void omap_pcie_init(void)
{
	debug(">>>pcie_init\n");

	/* Configure as RC mode */
	writel(2, CTRL_BASE + 0x0480);

	pcie_enable_module();

	pcie_pll_config();

	//pcie_hw_setup();

#if 0
	write_pcie_lcfg_reg_bits(STATUS_COMMAND,
				CFG_REG_CMD_STATUS_MEM_SPACE_ENB,
				CFG_REG_CMD_STATUS_MEM_SPACE);
#endif

	pcie_enable_link();

	ti_pcie_init_hoses();

	debug("<<<pcie_init\n");
}
