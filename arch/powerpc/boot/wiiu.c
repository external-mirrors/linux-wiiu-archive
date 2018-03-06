/*
 * arch/powerpc/boot/wiiu.c
 *
 * Nintendo Wii U
 * Copyright (C) 2017 Ash Logan <quarktheawesome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#include <stddef.h>
#include "string.h"
#include "stdio.h"
#include "types.h"
#include "io.h"
#include "ops.h"

BSS_STACK(8192);

/* If this struct ever has to be changed in a non-ABI compatible way,
   change the magic.
   Past magics:
    - 0xCAFEFECA: initial version
*/
#define WIIU_LOADER_MAGIC 0xCAFEFECA
struct wiiu_loader_data {
	unsigned int magic;
	char cmdline[256];
	void* initrd;
	unsigned int initrd_sz;
};
const static struct wiiu_loader_data* arm_data = (void*)0x89200000;

static void wiiu_copy_cmdline(char* cmdline, int cmdlineSz, unsigned int timeout) {
/*	If the ARM left us a commandline, copy it in */
	if (arm_data->magic == WIIU_LOADER_MAGIC) {
		strncpy(cmdline, arm_data->cmdline, 256);
	}
}

/* Mostly copied from gamecube.c. Obviously the GameCube is not the same
 * as the Wii U. TODO.
 */
void platform_init(unsigned int r3, unsigned int r4, unsigned int r5) {
	u32 heapsize = 16*1024*1024 - (u32)_end;
	simple_alloc_init(_end, heapsize, 32, 64);

	fdt_init(_dtb_start);

	console_ops.edit_cmdline = wiiu_copy_cmdline;
	if (arm_data->magic == WIIU_LOADER_MAGIC) {
		if (arm_data->initrd_sz > 0) {
			loader_info.initrd_addr = (unsigned long)arm_data->initrd;
			loader_info.initrd_size = (unsigned long)arm_data->initrd_sz;
		}
		//loader_info.cmdline = arm_data->cmdline;
	}
}
