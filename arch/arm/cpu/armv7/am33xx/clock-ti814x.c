/*
 * clock.c
 *
 * clocks for TI814X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

/*
 * Enable the peripheral clock for required peripherals
 */
static void enable_per_clocks(void)
{
	u32 temp;

	/* Selects OSC0 (20MHz) for DMTIMER1 */
	temp = readl(DMTIMER_CLKSRC);
	temp &= ~(0x7 << 3);
	temp |= (0x4 << 3);
	writel(temp, DMTIMER_CLKSRC);

	writel(0x2, DM_TIMER1_BASE + 0x54);
	while (readl(DM_TIMER1_BASE + 0x10) & 1)
		;

	writel(0x1, DM_TIMER1_BASE + 0x38);

	/* UARTs */
	writel(0x2, CM_ALWON_UART_0_CLKCTRL);
	while (readl(CM_ALWON_UART_0_CLKCTRL) != 0x2)
		;

	/* HSMMC */
	writel(0x2, CM_ALWON_HSMMC_CLKCTRL);
	while (readl(CM_ALWON_HSMMC_CLKCTRL) != 0x2)
		;
}

/*
 * select the HS1 or HS2 for DCO Freq
 * return : CLKCTRL
 */
static u32 pll_dco_freq_sel(u32 clkout_dco)
{
	if (clkout_dco >= DCO_HS2_MIN && clkout_dco < DCO_HS2_MAX)
		return SELFREQDCO_HS2;
	else if (clkout_dco >= DCO_HS1_MIN && clkout_dco < DCO_HS1_MAX)
		return SELFREQDCO_HS1;
	else
		return -1;

}

/*
 * select the sigma delta config
 * return: sigma delta val
 */
static u32 pll_sigma_delta_val(u32 clkout_dco)
{
	u32 sig_val = 0;
	float frac_div;

	frac_div = (float) clkout_dco / 250;
	frac_div = frac_div + 0.90;
	sig_val = (int)frac_div;
	sig_val = sig_val << 24;

	return sig_val;
}

/*
 * configure individual ADPLLJ
 */
static void pll_config(u32 base, u32 n, u32 m, u32 m2,
		       u32 clkctrl_val, int adpllj)
{
	u32 m2nval, mn2val, read_clkctrl = 0, clkout_dco = 0;
	u32 sig_val = 0, hs_mod = 0;

	m2nval = (m2 << 16) | n;
	mn2val = m;

	/* calculate clkout_dco */
	clkout_dco = ((OSC_0_FREQ / (n+1)) * m);

	/* sigma delta & Hs mode selection skip for ADPLLS*/
	if (adpllj) {
		sig_val = pll_sigma_delta_val(clkout_dco);
		hs_mod = pll_dco_freq_sel(clkout_dco);
	}

	/* by-pass pll */
	read_clkctrl = readl(base + ADPLLJ_CLKCTRL);
	writel((read_clkctrl | 0x00800000), (base + ADPLLJ_CLKCTRL));
	while ((readl(base + ADPLLJ_STATUS) & 0x101) != 0x101)
		;

	/* Clear TINITZ */
	read_clkctrl = readl(base + ADPLLJ_CLKCTRL);
	writel((read_clkctrl & 0xfffffffe), (base + ADPLLJ_CLKCTRL));

	/*
	 * ref_clk = 20/(n + 1);
	 * clkout_dco = ref_clk * m;
	 * clk_out = clkout_dco/m2;
	*/

	read_clkctrl = readl(base + ADPLLJ_CLKCTRL) & 0xffffe3ff;
	writel(m2nval, (base + ADPLLJ_M2NDIV));
	writel(mn2val, (base + ADPLLJ_MN2DIV));

	/* Skip for modena(ADPLLS) */
	if (adpllj) {
		writel(sig_val, (base + ADPLLJ_FRACDIV));
		writel((read_clkctrl | hs_mod), (base + ADPLLJ_CLKCTRL));
	}

	/* Load M2, N2 dividers of ADPLL */
	writel(0x1, (base + ADPLLJ_TENABLEDIV));
	writel(0x0, (base + ADPLLJ_TENABLEDIV));

	/* Load M, N dividers of ADPLL */
	writel(0x1, (base + ADPLLJ_TENABLE));
	writel(0x0, (base + ADPLLJ_TENABLE));

	/* Configure CLKDCOLDOEN,CLKOUTLDOEN,CLKOUT Enable BITS */
	read_clkctrl = readl(base + ADPLLJ_CLKCTRL) & 0xdfe5ffff;
	if (adpllj)
		writel((read_clkctrl | ADPLLJ_CLKCTRL_CLKDCO),
						base + ADPLLJ_CLKCTRL);

	/* Enable TINTZ and disable IDLE(PLL in Active & Locked Mode */
	read_clkctrl = readl(base + ADPLLJ_CLKCTRL) & 0xff7fffff;
	writel((read_clkctrl | 0x1), base + ADPLLJ_CLKCTRL);

	/* Wait for phase and freq lock */
	while ((readl(base + ADPLLJ_STATUS) & 0x600) != 0x600)
		;

}

static void unlock_pll_control_mmr(void)
{
	/* TRM 2.10.1.4 and 3.2.7-3.2.11 */
	writel(0x1EDA4C3D, 0x481C5040);
	writel(0x2FF1AC2B, 0x48140060);
	writel(0xF757FDC0, 0x48140064);
	writel(0xE2BC3A6D, 0x48140068);
	writel(0x1EBF131D, 0x4814006c);
	writel(0x6F361E05, 0x48140070);
}

static void mpu_pll_config(void)
{
	pll_config(MPU_PLL_BASE, MPU_N, MPU_M, MPU_M2, MPU_CLKCTRL, 0);
}

static void l3_pll_config(void)
{
	u32 l3_osc_src, rd_osc_src = 0;

	l3_osc_src = L3_OSC_SRC;
	rd_osc_src = readl(OSC_SRC_CTRL);

	if (OSC_SRC0 == l3_osc_src)
		writel((rd_osc_src & 0xfffffffe)|0x0, OSC_SRC_CTRL);
	else
		writel((rd_osc_src & 0xfffffffe)|0x1, OSC_SRC_CTRL);

	pll_config(L3_PLL_BASE, L3_N, L3_M, L3_M2, L3_CLKCTRL, 1);
}

void ddr_pll_config(unsigned int ddrpll_m)
{
	pll_config(DDR_PLL_BASE, DDR_N, DDR_M, DDR_M2, DDR_CLKCTRL, 1);
}

void enable_dmm_clocks(void)
{
	writel(0x2, CM_DEFAULT_FW_CLKCTRL);
	writel(0x2, CM_DEFAULT_L3_FAST_CLKSTCTRL);
	while ((readl(CM_DEFAULT_L3_FAST_CLKSTCTRL) & 0x300) != 0x300)
		;
	writel(0x2, CM_ALWON_L3_SLOW_CLKSTCTRL);
	while ((readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & 0x2100) != 0x2100)
		;
	writel(0x2, CM_DEFAULT_DMM_CLKCTRL);
	while ((readl(CM_DEFAULT_DMM_CLKCTRL)) != 0x2)
		;
}

void enable_emif_clocks(void)
{
	writel(0x2, CM_DEFAULT_EMIF_0_CLKCTRL);
	while ((readl(CM_DEFAULT_EMIF_0_CLKCTRL)) != 0x2)
		;
	writel(0x2, CM_DEFAULT_EMIF_1_CLKCTRL);
	while ((readl(CM_DEFAULT_EMIF_1_CLKCTRL)) != 0x2)
		;
}

/*
 * Configure the PLL/PRCM for necessary peripherals
 */
void pll_init()
{
	unlock_pll_control_mmr();

	/* Enable the control module */
	writel(0x2, CM_ALWON_CONTROL_CLKCTRL);

	mpu_pll_config();

	l3_pll_config();

	/* Enable the required peripherals */
	enable_per_clocks();
}
