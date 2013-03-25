cmake_minimum_required(VERSION 2.8.4)
project(UMD LPC1769 Library and Examples)
set(CMAKE_SYSTEM_NAME LPC1769)

set(CMAKE_LEGACY_CYGWIN_WIN32 0)

# the name of the target operating system
#
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)

# which compilers to use for C and C++
#
set(CMAKE_C_COMPILER       arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER     arm-none-eabi-g++)

SET(CMAKE_EXE_LINKER_FLAGS "-Xlinker --gc-sections")
SET(CMAKE_LINK_FLAGS)

set(COMPILE_DEFINITIONS_DEBUG    -O0 -g3 -DDEBUG)
set(COMPILE_DEFINITIONS_RELEASE  -O2)

add_definitions(
  -nostdlib

  -Wall
  -Wshadow
  -Wcast-qual
  -Wwrite-strings
  -Winline

  -std=gnu99

  -mcpu=cortex-m3
  -mthumb
)