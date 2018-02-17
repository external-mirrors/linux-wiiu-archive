/*
 * arch/powerpc/platforms/embedded6xx/latte-ahball-pic.h
 *
 * Nintendo Wii U "Latte" interrupt controller support
 * Copyright (C) 2018 Ash Logan <quarktheawesome@gmail.com>
 * Copyright (C) 2018 Roberto Van Eeden <rwrr0644@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#ifndef __LATTE_AHBALL_PIC_H
#define __LATTE_AHBALL_PIC_H

typedef struct __attribute__((packed)) {
	__be32 ahball_icr;		/* Triggered AHB IRQs (all) */
	__be32 ahblt_icr;		/* Triggered AHB IRQs (latte only) */
	__be32 ahball_imr;		/* Allowed AHB IRQs (all) */
	__be32 ahblt_imr;		/* Allowed AHB IRQs (latte only) */
} lt_pic_t;

#define LATTE_AHBALL_NR_IRQS    32

void latte_ahball_pic_probe(void);

#endif
