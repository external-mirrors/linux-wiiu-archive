/*
 * arch/powerpc/platforms/embedded6xx/wiiu/espresso-pic.c
 *
 * Nintendo Wii U "Latte" GPIO driver.
 * Copyright (C) 2018 Roberto Van Eeden <rwrr0644@gmail.com>
 *
 * Based on hlwd-gpio.c
 * Copyright (C) 2008-2009 The GameCube Linux Team
 * Copyright (C) 2008,2009 Albert Herranz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LATTE_GPIO_H
#define __LATTE_GPIO_H

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/irqdomain.h>

struct latte_gpio_chip {
	struct of_mm_gpio_chip gc;
	spinlock_t lock;
	struct irq_domain *irq_domain;
};

typedef struct __attribute__((packed)) {
	__be32 eout;		/* GPIO Outputs (Espresso access) */
	__be32 edir;		/* GPIO Direction (Espresso access) */
	__be32 ein;			/* GPIO Inputs (Espresso access) */
	__be32 eintlvl;		/* GPIO Interrupt Levels (Espresso access) */
	__be32 eintflag;	/* GPIO Interrupt Flags (Espresso access) */
	__be32 eintmask;	/* GPIO Interrupt Masks (Espresso access) */
	__be32 einmir;		/* GPIO Input Mirror (Espresso access) */
	__be32 enable;		/* GPIO Enable (Starbuck only) */
	__be32 out;			/* GPIO Outputs (Starbuck only) */
	__be32 dir;			/* GPIO Direction (Starbuck only) */
	__be32 in;			/* GPIO Inputs (Starbuck only) */
	__be32 intlvl;		/* GPIO Interrupt Levels (Starbuck only) */
	__be32 intflag;		/* GPIO Interrupt Flags (Starbuck only) */
	__be32 intmask;		/* GPIO Interrupt Masks (Starbuck only) */
	__be32 inmir;		/* GPIO Input Mirror (Starbuck only) */
	__be32 owner;		/* GPIO Owner Select (Starbuck only) */
} lt_gpio_t;


// Latte has 32 GPIOs for each controller
#define LATTE_GPIO_NR    32

// Initialize GPIO based interrupts
struct irq_domain *latte_gpio_pic_init(struct device_node *np, lt_gpio_t __iomem *regs);

#endif