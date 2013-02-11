/*
 * mux.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _MUX_H_
#define _MUX_H_

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_AM33XX
#include <asm/arch/mux-am335x.h>
#elif defined(CONFIG_TI814X)
#include <asm/arch/mux-ti814x.h>
#endif

struct module_pin_mux {
	short reg_offset;
	unsigned int val;
};

/* Pad control register offset */
#define PAD_CTRL_BASE	0x800
#define OFFSET(x)	(unsigned int) (&((struct pad_signals *) \
				(PAD_CTRL_BASE))->x)

/*
 * Configure the pin mux for the module
 */
void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux);

#endif
