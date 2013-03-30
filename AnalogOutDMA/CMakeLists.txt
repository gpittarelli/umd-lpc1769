cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ../Platform/LPC1769.cmake)

project(AnalogOutDMASounds C)

set(SOURCES
 src/cr_startup_lpc176x.c
 src/main.c
)

include_directories(../UMD_LPC1769/inc)

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/LPC1769.ld")

set(COMPILE_DEFINITIONS_DEBUG    -O0 -g3 -DDEBUG)
set(COMPILE_DEFINITIONS_RELEASE  -O2)

add_definitions(
  -D__NEWLIB__
  -D__CODE_RED
  -D__USE_CMSIS=CMSISv2p00_LPC17xx

  -O0
  -g3
  -DDEBUG

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
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(OUTPUT_NAME ${CMAKE_PROJECT_NAME}.axf)

add_executable(${OUTPUT_NAME} ${SOURCES})
set_target_properties(${OUTPUT_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${OUTPUT_NAME} CMSISv2p00_LPC17xx)

include(../Platform/LPC1769_targets.cmake)
