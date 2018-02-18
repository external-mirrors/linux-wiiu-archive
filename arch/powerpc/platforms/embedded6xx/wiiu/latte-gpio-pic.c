/*
 * arch/powerpc/platforms/embedded6xx/wiiu/latte-gpio-pic.c
 *
 * Nintendo Wii U GPIO interrupts.
 * Copyright (C) 2018 Roberto Van Eeden <rwrr0644@gmail.com>
 *
 * Based on hlwd-pic.c
 * Copyright (C) 2009 The GameCube Linux Team
 * Copyright (C) 2009 Albert Herranz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#define DRV_MODULE_NAME "latte-gpio-pic"
#define pr_fmt(fmt) DRV_MODULE_NAME ": " fmt

#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/io.h>
#include "latte-gpio.h"

/* IRQ chip operations
 */

static void latte_gpio_pic_mask_and_ack(struct irq_data *d) {
	lt_gpio_t __iomem *regs = irq_data_get_irq_chip_data(d);
	u32 mask = 1 << irqd_to_hwirq(d);
	out_be32(&regs->eintflag, mask);
	clrbits32(&regs->eintmask, mask);
}

static void latte_gpio_pic_ack(struct irq_data *d) {
	lt_gpio_t __iomem *regs = irq_data_get_irq_chip_data(d);
	out_be32(&regs->eintflag, 1 << irqd_to_hwirq(d));
}

static void latte_gpio_pic_mask(struct irq_data *d) {
	lt_gpio_t __iomem *regs = irq_data_get_irq_chip_data(d);
	clrbits32(&regs->eintmask, 1 << irqd_to_hwirq(d));
}

static void latte_gpio_pic_unmask(struct irq_data *d) {
	lt_gpio_t __iomem *regs = irq_data_get_irq_chip_data(d);
	setbits32(&regs->eintmask, 1 << irqd_to_hwirq(d));
}

static struct irq_chip latte_gpio_pic = {
	.name			= "latte-gpio-pic",
	.irq_ack		= latte_gpio_pic_ack,
	.irq_mask_ack	= latte_gpio_pic_mask_and_ack,
	.irq_mask		= latte_gpio_pic_mask,
	.irq_unmask		= latte_gpio_pic_unmask,
};

/* Domain Ops
 */

static int latte_gpio_pic_match(struct irq_domain *h, struct device_node *node, enum irq_domain_bus_token bus_token) {
	if (h->fwnode == &node->fwnode) {
		pr_debug("%s IRQ matches with this driver\n", node->name);
		return 1;
	}
	return 0;
}

static int latte_gpio_pic_alloc(struct irq_domain *h, unsigned int virq, unsigned int nr_irqs, void *arg) {
	//See espresso-pic for slight elaboration
	unsigned int i;
	struct irq_fwspec* fwspec = arg;
	irq_hw_number_t hwirq = fwspec->param[0];
	
	for (i = 0; i < nr_irqs; i++) {
		irq_set_chip_data(virq + i, h->host_data);
		irq_set_status_flags(virq + i, IRQ_LEVEL);
		irq_set_chip_and_handler(virq + i, &latte_gpio_pic, handle_level_irq);
		irq_domain_set_hwirq_and_chip(h, virq + i, hwirq + i, &latte_gpio_pic, h->host_data);
	}
	return 0;
}

static void latte_gpio_pic_free(struct irq_domain *h, unsigned int virq, unsigned int nr_irqs) {
	struct irq_data *data = irq_domain_get_irq_data(h, virq);
	
	irq_domain_reset_irq_data(data);
	pr_debug("free\n");
}

const struct irq_domain_ops latte_gpio_pic_ops = {
	.match = latte_gpio_pic_match,
	.alloc = latte_gpio_pic_alloc,
	.free = latte_gpio_pic_free,
};

/* Determinate if there are interrupts pending
 */
unsigned int latte_gpio_pic_get_irq(struct irq_domain *h) {
	lt_gpio_t __iomem *regs = h->host_data;
	u32 irq_status, irq;
	
	//Get IRQ status
	irq_status = in_be32(&regs->eintmask) & in_be32(&regs->eintflag);
	
	if (irq_status == 0)
		return 0;	//No IRQs pending
	
	//Find the first IRQ
	irq = __ffs(irq_status);

	//Return the virtual IRQ
	return irq_linear_revmap(h, irq);
}

/* Cascade IRQ handler
 */
static void latte_gpio_irq_cascade(struct irq_desc *desc) {
	struct irq_domain *irq_domain = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	unsigned int virq;
	
	raw_spin_lock(&desc->lock);
	chip->irq_mask(&desc->irq_data); /* IRQ_LEVEL */
	raw_spin_unlock(&desc->lock);
		
	virq = latte_gpio_pic_get_irq(irq_domain);
	if (virq)
		generic_handle_irq(virq);
	else
		pr_err("spurious interrupt!\n");

	raw_spin_lock(&desc->lock);
	chip->irq_ack(&desc->irq_data); /* IRQ_LEVEL */
	if (!irqd_irq_disabled(&desc->irq_data) && chip->irq_unmask)
		chip->irq_unmask(&desc->irq_data);
	raw_spin_unlock(&desc->lock);
}

/* Init function
 */
struct irq_domain *latte_gpio_pic_init(struct device_node *np, lt_gpio_t __iomem *regs) {
	int irq_cascade;	
	struct irq_domain *irq_domain;

	//Check if the driver is valid
	if(!of_get_property(np, "interrupts", NULL)) {
		pr_err("no cascade interrupt specified\n");
		return NULL;
	}
	
	//Mask and Ack all IRQs
	out_be32(&regs->eintmask, 0);
	out_be32(&regs->eintflag, 0xffffffff);
	
	//Register PIC
	irq_domain = irq_domain_add_linear(np, LATTE_GPIO_NR, &latte_gpio_pic_ops, regs);
	if (!irq_domain) {
		pr_err("failed to add irq domain\n");
		return NULL;
	}
	
	//Setup cascade interrupt
	irq_cascade = irq_of_parse_and_map(np, 0);
	irq_set_chained_handler_and_data(irq_cascade, latte_gpio_irq_cascade, irq_domain);
	
	//Success
	pr_info("successfully initialized");

	return irq_domain;
}