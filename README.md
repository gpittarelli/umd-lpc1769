UMD LPC1796 Library
======

This repository contains a simple library for the NXP LPC1769
microcontroller with example projects. Building and running this
project (on the microcontroller) requires an installation of
LPCXpresso which includes the ARM GCC Toolchain, NXP's LPC Library,
and a command line utility to flash programs to the chip.

CMake is used to allow building of this repository in many different
operating systems and development environments.

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
name in the root of this repository.

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
directory, and then you can run LPCXpresso's flash utilities and gdb
using your chosen build system (See 'Available Targets').

The `Skeleton` directory inclues an example project which can be
copied to create a new project.

Available Targets
======

The default build target builds a .axf file with debug settings.

The following targets are provided:

  * `lst` Generate a .lst file, which includes an overview of all
    the sections and symbols in your output, along with a complete
    disassembly.
  * `hex` and `bin` Generate alternate formats of the normal .axf
    output, which may be useful if you want to use other tools.
  * `boot` Boot the LPC-Link board. This is required before flashing
    or debugging the microcontroller using the LPC-Link board.
  * `flash` Write the currently built output to the
    microcontroller. The microcontroller immediately starts running
    after the flash is complete.
  * `flash-halt` The same as `flash`, but the microcontroller is left
    in a stopped state.
  * `gdb` Launch a gdb session. Complete debug information is also
    setup. The gdb server provided by LPCXpresso does not support the
    required setting to allow `run` (`r`) to restart the program from
    the correct entry point. Instead, `run` results in a hard
    fault. To restart execution, you should instead type `info files`
    to find the entry point of the current image, and then jump to the
    entry point with `jump` (`j`). You can also use the `load`
    command, however that will flash the entire image to the chip
    again, which may take a while for projects with large compiled
    binaries.

Command Overview
======

For reference, the commands called by the toolchain look like:

GCC
