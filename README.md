UMD LPC1796 Library
======

This repository contains a simple library for the NXP LPC1769
microcontroller with example projects. Building and running this
project (on the microcontroller) requires an installation of
LPCXpresso which includes the ARM GCC Toolchain, NXP's LPC Library,
and a command line utility to flash programs to the chip.

CMake is used to allow building of this repository in many different
operating systems and devleopment environments.

Developing in NXP's LPCXpresso IDE for the LPC1769 is not required,
however certain proprietary utilities are required for flashing the
chip through the LPC-Link board. If you want to work with the board
with no proprietary software, the LPC-Link board can be chopped off
and the remaining board with the LPC1769 can be programmed via JTAG.

The code base also requires an LPCXpresso install for the provided
CMSIS library and cr\_startup\_lpc176x.c.

Setup
======

Extract and copy CMSISv2p00_LPC17xx.zip into the folder with the same
name in the root of this repository. Also find cr\_startup\_lpc176x.c
and copy it into each project you want to compile.

First have CMake installed (version >= 2.4.8), and LPCXpresso (tested
with version 5 installed, but should work with any version with the
same toolchain) installed.

Then open `LPCXpressoDir.cmake` in the root directory and update
LPCXPRESSO_DIR to point to the root of your LPCXpresso installation.

Next, run CMake in both the CMSISv2p00\_LPC17xx directory and the
UMD\_LPC1769 directory:

    cmake . -G "Unix Makefiles"

The parameters are:

  * `.` The directory to run CMake in.
  * `-G "Unix Makefiles"` Specifies your desired build system. Run
    cmake with no parameters to get a list of all available build
    system targets.

Then to use a project, run CMake as above in the selected project
diretory, and then you can run LPCXpresso's flash utilities and gdb
using your chosen build system.
