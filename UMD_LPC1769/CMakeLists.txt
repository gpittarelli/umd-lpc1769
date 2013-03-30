cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ../Platform/LPC1769.cmake)

project(UMD_LPC1769 C)

set(SOURCES
 src/PLL.c
)

include_directories(inc)

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3 -mthumb")

set(COMPILE_DEFINITIONS_DEBUG    -O0 -g3 -DDEBUG)
set(COMPILE_DEFINITIONS_RELEASE  -O2)

add_definitions(
  -D__REDLIB__
  -D__CODE_RED

  -Wall
  -Wshadow
  -Wcast-qual
  -Wwrite-strings
  -Winline

  -fmessage-length=80
  -ffunction-sections
  -fdata-sections

  -mcpu=cortex-m3
  -mthumb
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
