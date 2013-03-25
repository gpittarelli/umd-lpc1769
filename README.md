UMD LPC1796 Library
======

This repository contains a simple library for the NXP LPC1769 microcontroller with example projects. Building and running this project (on the microcontroller) requires an installation of LPCXpresso which includes the ARM GCC Toolchain, NXP's LPC Library, and a command line utility to flash programs to the chip.

Developing in NXP's LPCXpresso IDE for the LPC1769 is not required, however certain proprietary utilities are required for flashing the chip through the LPC-Link board. If you want to work with the board with no proprietary software, the LPC-Link board can be chopped off and the remaining board with the LPC1769 can be programmed via JTAG with the same binaries.

The code base also requires an LPCXpresso install for the provided CMSIS library and cr\_startup\_lpc176x.c.

Setup
======

Extract and copy CMSISv2p00_LPC17xx.zip into a folder with the same name in the root of this repository. Also find cr\_startup\_lpc176x.c and copy it into each project you want to compile.

TODO: Setup CMake to share default startup file.
