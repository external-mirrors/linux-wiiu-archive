/*
 * arch/powerpc/platforms/embedded6xx/wiiu/latte-gpio.c
 *
 * Nintendo Wii U "Latte" GPIO driver
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
 *
 */

#define DRV_MODULE_NAME "latte-gpio"
#define pr_fmt(fmt) DRV_MODULE_NAME ": " fmt

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/irqdomain.h>
#include "latte-gpio.h"

static inline struct latte_gpio_chip *
to_latte_gpio_chip(struct of_mm_gpio_chip *mm_gc)
{
	return container_of(mm_gc, struct latte_gpio_chip, gc);
}

static int latte_gpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	lt_gpio_t __iomem *regs = mm_gc->regs;
	unsigned int val;

	val = (in_be32(&regs->in) & (1 << gpio)) != 0;

	pr_info("%s: gpio: %d val: %d\n", __func__, gpio, val);

	return val;
}

static void latte_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct latte_gpio_chip *lt_gc = to_latte_gpio_chip(mm_gc);
	lt_gpio_t __iomem *regs = mm_gc->regs;
	u32 pin_mask = 1 << gpio;
	u32 data;
	unsigned long flags;

	spin_lock_irqsave(&lt_gc->lock, flags);
	data = in_be32(&regs->in) & ~pin_mask;
	if (val)
		data |= pin_mask;
	out_be32(&regs->out, data);
	spin_unlock_irqrestore(&lt_gc->lock, flags);

	pr_info("%s: gpio: %d val: %d\n", __func__, gpio, val);
}

static int latte_gpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	lt_gpio_t __iomem *regs = mm_gc->regs;
	u32 pin_mask = 1 << gpio;

	setbits32(&regs->enable, pin_mask);
	clrbits32(&regs->dir, pin_mask);

	return 0;
}

static int latte_gpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	lt_gpio_t __iomem *regs = mm_gc->regs;
	u32 pin_mask = 1 << gpio;

	setbits32(&regs->enable, pin_mask);
	setbits32(&regs->dir, pin_mask);
	latte_gpio_set(gc, gpio, val);

	return 0;
}

static int latte_gpio_to_irq(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct latte_gpio_chip *lt_gc = to_latte_gpio_chip(mm_gc);
	
	if(lt_gc->irq_domain) {
		//Don't look at this code, it hurts
		struct irq_fwspec f;
		f.param[0] = gpio;
		return irq_domain_alloc_irqs(lt_gc->irq_domain, 1, NUMA_NO_NODE, &f);
	} else {
		return -ENXIO;
	}
}

void latte_gpio_add32(struct device_node *np) {
	struct of_mm_gpio_chip *mm_gc;
	struct latte_gpio_chip *lt_gc;
	struct gpio_chip *gc;
	lt_gpio_t __iomem *regs;
	
	lt_gc = kzalloc(sizeof(*lt_gc), GFP_KERNEL);
	if (!lt_gc) {
		pr_err("error allocating chip data");
		return;
	}

	spin_lock_init(&lt_gc->lock);
	
	mm_gc = &lt_gc->gc;
	gc = &mm_gc->gc;

	gc->ngpio = 32;
	gc->direction_input = latte_gpio_dir_in;
	gc->direction_output = latte_gpio_dir_out;
	gc->get = latte_gpio_get;
	gc->set = latte_gpio_set;
	gc->to_irq = latte_gpio_to_irq;
	
	if (of_mm_gpiochip_add(np, mm_gc) != 0) {
		pr_err("error adding gpio chip");
		return;
	}
	
	regs = mm_gc->regs;
	pr_info("added %u gpios at %p\n", gc->ngpio, regs);
	
	out_be32(&regs->owner, 0);
	lt_gc->irq_domain = latte_gpio_pic_init(np, regs);
}

static int latte_gpio_init(void) {
	struct device_node *np;

	for_each_compatible_node(np, NULL, "nintendo,latte-gpio")
		latte_gpio_add32(np);
	
	return 0;
}
arch_initcall(latte_gpio_init);
