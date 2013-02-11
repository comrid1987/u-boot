/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _CLOCKS_TI814X_H_
#define _CLOCKS_TI814X_H_

#define DDR_PLL_400		/* Values supported 400,533 */

/* CLK_SRC */
#define OSC_SRC0		0
#define OSC_SRC1		1

#define L3_OSC_SRC		OSC_SRC0

#define OSC_0_FREQ		20

#define DCO_HS2_MIN		500
#define DCO_HS2_MAX		1000
#define DCO_HS1_MIN		1000
#define DCO_HS1_MAX		2000

#define SELFREQDCO_HS2		0x00000801
#define SELFREQDCO_HS1		0x00001001

#define MPU_N			0x1
#define MPU_M			0x3C
#define MPU_M2			1
#define MPU_CLKCTRL		0x1

#define L3_N			19
#define L3_M			880
#define L3_M2			4
#define L3_CLKCTRL		0x801

#define DDR_N			19
#define DDR_M			666
#define DDR_M2			2
#define DDR_CLKCTRL		0x801

/* Clocks are derived from ADPLLJ */
#define ADPLLJ_CLKCTRL		0x4
#define ADPLLJ_TENABLE		0x8
#define ADPLLJ_TENABLEDIV	0xC
#define ADPLLJ_M2NDIV		0x10
#define ADPLLJ_MN2DIV		0x14
#define ADPLLJ_FRACDIV		0x18
#define ADPLLJ_STATUS		0x24

/* ADPLLJ register values */
#define ADPLLJ_CLKCTRL_HS2	0x00000801 /* HS2 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_HS1	0x00001001 /* HS1 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_CLKDCO	0x201A0000

#define MPU_PLL_BASE			(PLL_SUBSYS_BASE + 0x048)
#define L3_PLL_BASE			(PLL_SUBSYS_BASE + 0x110)
#define USB_PLL_BASE			(PLL_SUBSYS_BASE + 0x260)
#define DDR_PLL_BASE			(PLL_SUBSYS_BASE + 0x290)

#define OSC_SRC_CTRL			(PLL_SUBSYS_BASE + 0x2C0)
#define OSC_SRC				(PLL_SUBSYS_BASE + 0x2C0)
#define ARM_CLKSRC			(PLL_SUBSYS_BASE + 0x2C4)
#define MLB_ATL_CLKSRC			(PLL_SUBSYS_BASE + 0x2CC)
#define DMTIMER_CLKSRC			(PLL_SUBSYS_BASE + 0x2E0)
#define CLKOUT_MUX			(PLL_SUBSYS_BASE + 0x2E4)
#define SYSCLK18_SRC			(PLL_SUBSYS_BASE + 0x2F0)
#define WDT0_CLKSRC			(PLL_SUBSYS_BASE + 0x2F4)

/* PRCM */
#define CM_DPLL_OFFSET			(PRCM_BASE + 0x0300)

/*EMIF4 PRCM Defintion*/
#define CM_DEFAULT_L3_FAST_CLKSTCTRL	(PRCM_BASE + 0x0508)
#define CM_DEFAULT_EMIF_0_CLKCTRL	(PRCM_BASE + 0x0520)
#define CM_DEFAULT_EMIF_1_CLKCTRL	(PRCM_BASE + 0x0524)
#define CM_DEFAULT_DMM_CLKCTRL		(PRCM_BASE + 0x0528)
#define CM_DEFAULT_FW_CLKCTRL		(PRCM_BASE + 0x052C)

#define CM_ALWON_L3_SLOW_CLKSTCTRL	(PRCM_BASE + 0x1400)
#define CM_ALWON_UART_0_CLKCTRL		(PRCM_BASE + 0x1550)
#define CM_ALWON_UART_1_CLKCTRL		(PRCM_BASE + 0x1554)
#define CM_ALWON_UART_2_CLKCTRL		(PRCM_BASE + 0x1558)
#define CM_ALWON_GPIO_0_CLKCTRL		(PRCM_BASE + 0x155c)
#define CM_ALWON_GPIO_0_OPTFCLKEN_DBCLK (PRCM_BASE + 0x155c)
#define CM_ALWON_WDTIMER_CLKCTRL	(PRCM_BASE + 0x158C)
#define CM_ALWON_SPI_CLKCTRL		(PRCM_BASE + 0x1590)
#define CM_ALWON_CONTROL_CLKCTRL	(PRCM_BASE + 0x15C4)
#define CM_ALWON_HSMMC_CLKCTRL		(PRCM_BASE + 0x1620)

#define CM_DLL_CTRL_NO_OVERRIDE		0

extern void pll_init(void);
extern void enable_emif_clocks(void);
extern void enable_dmm_clocks(void);

#endif	/* endif _CLOCKS_TI814X_H_ */
