UMD LPC1796 Examples
======

This repositoty contains a CMake build system, a simple example
library (UMDLPC) and a number of example projects for the LPC1769.

CMake (a sort of build system that produces other build systems) is
used to allow building of this repository in many different operating
systems and development environments.

Developing in NXP's LPCXpresso IDE for the LPC1769 is not required,
however the IDE must be installed for the arm-none-eabi-* toolchain,
certain proprietary utilities required for communicating with the
LPC-Link board, and also the CMSISv2p00_LPC17xx library.

All of these dependencies could be dropped, by getting the
arm-none-eabi-* toolchain from another source, chopping off the
LPC-Link board and programming the chip with JTAG instead, and
including the CMSIS library in the repo (or using a different
library).

  * [Setup](#setup)
  * [Targets](#targets)
  * [Typical Workflow](#typical-workflows)
  * [Command Overview](#command-overview)

Setup
-------

First have CMake (version >= 2.4.8), and LPCXpresso (version 5)
installed.

 1. First clone this repository.

 2. In a terminal in the `_setup` directory of this repository, run:

        cmake . -DLPCXPRESSO_DIR=<lpcxpresso_dir>

  Where `lpcxpresso_dir` is the root directory of your LPCXpresso
  installation (eg `/usr/local/lpcxpresso_5.1.2_2065/lpcxpresso`).

 3. Extract the `CMSISv2p00_LPC17xx.zip` file into the folder of the
    same name.

 4. Genreate a build system for CMSIS and UMDLPC libraries, by running
    CMake in both the `CMSISv2p00_LPC17xx` directory and the
    `UMD_LPC1769` directories:

        cmake . -G "Unix Makefiles"

    The parameters are:
    * `.` The directory to run CMake in.
    * `-G "Unix Makefiles"` Specifies your desired build system. Run
      cmake with no parameters to get a list of all available build
      system targets.

 5. Build the CMSIS and UMDLPC libraries using your chosen build
    system. With makefiles, just run make in both directories.

To use a project, run CMake as above in the selected project
directory, build the project with your chosen build system, and then
you can use LPCXpresso's flash utilities and gdb with your chosen
build system (See [Targets](#targets)).

The `Skeleton` directory inclues an example project which can be
copied to create new projects. Remember to update the `CMakeLists.txt`
file in each new project to reflect the new project's name.

Overall, a full install, library builds and build of a project (for
example, AnalogOutDMA) should look like:

    $ # Download project
    $ git clone git://github.com/gpittarelli/umd-lpc1769.git
    $ # Run setup script
    $ cd umd-lpc1769/_setup
    $ cmake . -DLPCXPRESSO_DIR=/usr/local/lpcxpresso_5.1.2_2065/lpcxpresso/
    $ # Extract CMSIS library .zip
    $ cd ..
    $ unzip CMSISv2p00_LPC17xx.zip -d CMSISv2p00_LPC17xx/
    $ # Build CMSIS
    $ cd CMSISv2p00_LPC17xx/
    $ cmake . -G "Unix Makefiles"
    $ make
    $ # Build UMDLPC library
    $ cd ../UMD_LPC1769/
    $ cmake . -G "Unix Makefiles"
    $ make
    $ # We're finally ready to build a project:
    $ cd ..
    $ cd AnalogOutDMA/
    $ cmake . -G "Unix Makefiles"
    $ make
    $ # Rebuilds do not require running cmake again, just make

Targets
------

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
    setup. Note that it is not always simple to restart the current
    program. `run` (`r`) will work for some programs, and sometimes,
    using `jump` (`j`) to jump to the entry point of the current image
    (found with `info files`) works. However, none of these completely
    reset the chip - the only command that will is the `load` command,
    however that will flash the entire image to the chip again, which
    may take a while for projects with large compiled binaries. Also
    note that some circuits (particularly when interfacing with other
    chips that have their own state) may require completely unplugging
    the circuit between program runs.

If using makefiles, these are directly accessible as `make lst`,
etc. run in the root directory of the desired project.

Typical Workflows
------

These sections detail how to do many common tasks. They are written
under the assumption that a Makefile build system is being used, but
the calls to `make` can all be substituted with the appropriate action
in any other chosen build system.

### Start a new project:

    $ cp -R Skeleton/ NewProjectName
    $ cd NewProjectName
    $ mv src/main.c src/newproject.c
    $ # Open CMakeLists.txt and change project name, and update
        main.c in SOURCES list
    $ cmake . -G "Unix Makfiles"
    $ make

### Work on an existing project

    $ # Open source files in editor and make+save changes
    $ make

### Enabling semihosting for a project

Open a project's CMakeLists.txt file and uncomment the following line:

    set(SEMIHOSTING_ENABLED True)

Then just rebuild the project, and semihosting messages will be
viewable in gdb.

### Adding compiler options

All compiler options are configured in
`Platform/LPC1769_project_default.cmake`. If you to add compiler
options to a single project, you can use CMake's `add_definitions`
command in that project's CMakeLists.txt.

### Add a source file to a project

When adding source files to a project, remember to open that projects
CMakeLists.txt and add the file to the `SOURCES` variable.

    set(SOURCES
      src/cr_startup_lpc176x.c
      src/project.c
      # Add more source files here
    )

### Flash a program

    $ # Plug in the LPC1769
    $ make
    $ make boot
    $ make flash

### Debug a program

    $ # Plug in the LPC1769
    $ make
    $ make boot
    $ make gdb
    (gdb) b main
    (gdb) load
    (gdb) c

    # Make changes to the program

    (gdb) make
    (gdb) load
    (gdb) c

Command Overview
------

For reference, the commands called by the build system are described
below.

### Compiling

Compiling and linking are done with `arm-none-eabi-gcc`.

Compiler flags:

    -D__CODE_RED
    -D__NEWLIB__
    -D__USE_CMSIS=CMSISv2p00_LPC17xx
    -I..\CMSISv2p00_LPC17xx\inc
    -I..\UMD_LPC1769\inc
    -Wall
    -Wshadow
    -Wcast-qual
    -Wwrite-strings
    -Winline
    -fmessage-length=80
    -ffunction-sections
    -fdata-sections
    -std=gnu99
    -mcpu=cortex-m3
    -mthumb

To specify source input and object output:
    -o out.obj -c in.c

Extra flags for debugging:

    -DDEBUG
    -g
    -O0

Extra flags for release build:

    -O2 -Os

### Linking

Linker flags:

    -nostdlib
    -Xlinker
    --gc-sections
    -mcpu=cortex-m3
    -mthumb
    -T ../Platform/LPC1769.ld

Note the linker script `LPC1769.ld` in the `Platform` directory, which
can be exchanged for `LPC1769_semihosting.ld` in order to build
against the Newlib library for semihosting.

### Booting the LPC-Link

Before communicating with the LPC-Link board, the board has to be
'booted' using the `dfu-util` tool:

    dfu-util -d 0471:df55 -c 0 -t 2048 -R -D LPCXpressoWIN.enc

Where LPCXpressoWIN.enc is a file in LPCXpresso's bin dir.

On windows, a LPCXpresso provides a `bootLPCXpresso.cmd` script in the
bin/Scripts directory which also boots the LPC-Link.

### Separating debug symbols

Debug files can be stripped from the created `.axf` executable and
placed in a separate file.

First `objcopy` is used to copy only the debug symbols to a new file:

    arm-none-eabi-objcopy --only-keep-debug in.axf out.axf.debug

Next `strip` is used to remove the debug symbols from the original
file:

    arm-none-eabi-strip -g in.axf

### Converting output type

To create a HEX file

    objdump -Ohex in.axf out.bin

To create a BIN file

    objdump -Obinary in.axf out.bin

### Creating a disassembly listing

    objdump -x -D in.axf > out.lst

### Debugging

The debugger can be run two ways, with the gdb server inernally or externally:

    arm-none-eabi-gdb --exec="in.axf" --symbols="in.axf.debug"

Next the following commands are run:

    set remotetimeout 5000
    set mem inaccessible-by-default off
    mem ondisconnect cont
    set arm force-mode thumb
    target extended-remote | crt_emu_cm3_nxp -2 -g -wire=<wire_type> \
                                      -pLPC1769 -vendor=NXP
    mon semihosting ena

`wire_type` is `hid` on Windows 7, and `winusb` on most other systems
(older Windows versions, Linux, etc.)

These commands can be automated by placing them in a script file and
specifying it with `--command=<script_file>`, or running each command
with `--eval-command=<command>`.
