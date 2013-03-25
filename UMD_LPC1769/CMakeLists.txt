cmake_minimum_required(VERSION 2.8.4)
project (UMD LPC1769 Library)

set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

FILE(GLOB MyCSources
  ${PROJECT_SOURCE_DIR}/*.c
  ${PROJECT_SOURCE_DIR}/*.h
)


add_executable(${CMAKE_PROJECT_NAME}.elf ${exmpl_sources})

#Libraries
target_link_libraries(${CMAKE_PROJECT_NAME}.elf

  ${PERIPH_LIBRARY_NAME}
)

add_custom_target(${CMAKE_PROJECT_NAME}.hex DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex)
add_custom_target(${CMAKE_PROJECT_NAME}.bin DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin)
add_custom_target(${CMAKE_PROJECT_NAME}.lst DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJDUMP} -D ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.lst)