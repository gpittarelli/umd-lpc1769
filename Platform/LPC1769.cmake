include(../LPCXpressoDir.cmake)

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED LPCXPRESSO_DIR)
  message( FATAL_ERROR "******************************
Must define LPCXPRESSO_DIR pointing to LPCXpresso install directory. Look in LPCXpressoDir.cmake or run cmake with -DLPCXPRESSO_DIR=<lpcxpresso_dir>.
******************************
")
endif()

message(STATUS "Using LPCXpresso Directory: ${LPCXPRESSO_DIR}")
file(TO_CMAKE_PATH ${LPCXPRESSO_DIR} LPCXPRESSO_DIR)
set(LPCXPRESSO_GNU_DIR ${LPCXPRESSO_DIR}/tools/bin)
set(LPCXPRESSO_BIN_DIR ${LPCXPRESSO_DIR}/bin)
message(STATUS "GNU Toolchain at: ${LPCXPRESSO_GNU_DIR}")
message(STATUS "LPCXpresso utilities at: ${LPCXPRESSO_BIN_DIR}")

# which compilers to use for C and C++
find_program(CMAKE_C_COMPILER arm-none-eabi-gcc ${LPCXPRESSO_GNU_DIR}
             NO_DEFAULT_PATH)
#find_program(CMAKE_C_LINK_EXECUTABLE arm-none-eabi-ld ${LPCXPRESSO_GNU_DIR})
message(STATUS "ARM C compiler: ${CMAKE_C_COMPILER}")

# Compilers like arm-none-eabi-gcc that target bare metal systems don't pass
# CMake's compiler check, so fill in the results manually and mark the test
# as passed:
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_COMPILER_IS_GNUCC 1)
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)

include_directories(../CMSISv2p00_LPC17xx/inc)
link_directories(../CMSISv2p00_LPC17xx/lib)
