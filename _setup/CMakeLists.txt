cmake_minimum_required(VERSION 2.8.4)

if(NOT DEFINED LPCXPRESSO_DIR)
  message( FATAL_ERROR "******************************
Please define LPCXPRESSO_DIR pointing to your LPCXpresso install directory.
Run cmake with -DLPCXPRESSO_DIR=<lpcxpresso_dir>
******************************")

endif()

file(TO_CMAKE_PATH ${LPCXPRESSO_DIR} LPCXPRESSO_DIR)

set(CMSIS_ZIP_PATH "Examples/NXP/LPC1000/LPC17xx/CMSISv2p00_LPC17xx.zip")
file(
  COPY "${LPCXPRESSO_DIR}/${CMSIS_ZIP_PATH}"
  DESTINATION "."
)

file(WRITE "../LPCXpressoDir.cmake"
"\# Auto generated file
set(LPCXPRESSO_DIR \"${LPCXPRESSO_DIR}\")
")
