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

#define CMDLINE_CANARY_LOC (unsigned int*)0x89200000
#define CMDLINE_CANARY_MAGIC (unsigned int)0xCAFEFECA
#define CMDLINE_LOC (char*)0x89200004
static void wiiu_copy_cmdline(char* cmdline, int cmdlineSz, unsigned int timeout) {
/*	If the ARM left us a commandline, copy it in */
	if (*CMDLINE_CANARY_LOC == CMDLINE_CANARY_MAGIC) {
		strncpy(cmdline, CMDLINE_LOC, 256);
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
}
