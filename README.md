# Wii U Linux
**A port of Linux 4.11.6 to the Wii U baremetal.**

[![pipeline status](https://gitlab.com/linux-wiiu/linux-wiiu/badges/master/pipeline.svg)](https://gitlab.com/linux-wiiu/linux-wiiu/commits/master)

### Compiling
Assuming you have devkitPPC (though any PowerPC compiler will work:)
```sh
#clone repo however you'd like
cp arch/powerpc/configs/wiiu_defconfig .config
#you don't need to make any changes, just enter and exit to generate the config
make menuconfig ARCH=powerpc CROSS_COMPILE=powerpc-eabi- CROSS32_COMPILE=powerpc-eabi-
#add -j4 if you'd like
make ARCH=powerpc CROSS_COMPILE=powerpc-eabi- CROSS32_COMPILE=powerpc-eabi-
```
This'll build `arch/powerpc/boot/dtbImage.wiiu`. From here, check on [linux-wiiu/linux-loader](https://gitlab.com/linux-wiiu/linux-loader) for your next steps. There are no kernel modules to worry about, unless you build your own.

You can also download a precompiled dtbImage of the latest git [here](https://gitlab.com/linux-wiiu/linux-wiiu/-/jobs/artifacts/master/raw/dtbImage.wiiu?job=master-build).

### Booting
The kernel commandline is hardcoded (for now) with `root=sda1 rootwait`. This means you'll need to use a USB flash drive as your rootfs. Format it however you'd like (yes, ext4/gpt works) and throw a distro on it. Plug it and a USB keyboard into the Wii U. Run [linux-wiiu/linux-loader](https://gitlab.com/linux-wiiu/linux-loader) (as described in that repo's README) and enjoy your Linux!

*TODO: pick a distro to support, get SD card booting user-accessible*

### Device Support
As it stands, we've got:
 - USB OHCI/EHCI (back ports only)
 - Framebuffer graphics (no acceleration or DRI)
 - SD card
 - Bluetooth (tether a phone to get internet)
 - Some ARM interaction (for poweroff/reboot)
 - 2GB of RAM (0x80000000, reclaimed from ARM)
 - One PowerPC core (core 0?)
 - Both interrupt controllers (this is a big deal for us)

Some of our TODOs can be found on [the boards](https://gitlab.com/linux-wiiu/linux-wiiu/boards).

### Distributions and Programs
While we started off developing with Gentoo, we swapped to Debian unstable (everything's precompiled) so that's what we reccomend you do too. Debain stable/testing is unbearably outdated on PowerPC so yes, you should use sid/unstable. To make a system, you can use debootstrap or our prebuilt option:

1. Be on Linux. Get a USB (512mb bare minimum) and format it with a single ext4 partition. This will be your rootfs, so make sure it's a decent quality one (speed is important)
2. Download [this archive](https://mega.nz/#!la52GDSS!Y9TnuFmvbWRbFZww7LPvVsyh2egz4CTDyxC2R5r62r4), we'll call it "debian.tar.xz"
3. Mount and cd into your new USB.
4. Run `tar -xvpf <path/to/debian.tar.xz>`. The p is important, you could skip the v.
5. Eject the drive and plug it into your Wii U. With any luck, it'll boot Debian! Log in with username root and password root.

##### Notes
 - This version of Debian is set up to keep the kernel up to date - it'll mount the SD whenever it does this. Remove `deb.heyquark.com` from the apt sources to disable this.
 - No, we haven't tried X.org. It might work with `xf86-video-fbdev`, albeit *slowly*.
 - If you can come up with a better guide (esp. one including steps for Windows users) feel free to PR it in.
