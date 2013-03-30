add_custom_target(hex
  DEPENDS ${OUTPUT_NAME}
  COMMAND ${CMAKE_OBJCOPY} -Oihex ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}
                ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex)

add_custom_target(bin
  DEPENDS ${OUTPUT_NAME}
  COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}
                ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin)

add_custom_target(lst
  DEPENDS ${OUTPUT_NAME}
  COMMAND ${CMAKE_OBJDUMP} -x -D ${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_NAME}
                       > ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.lst)

if(${CMAKE_HOST_WIN32})

  set(LPCXPRESSO_WIRE hid)

  add_custom_target(boot
    COMMAND ${LPCXPRESSO_BIN_DIR}/Scripts/bootLPCXpresso.cmd hid)

else()

  set(LPCXPRESSO_WIRE winusb)

  add_custom_target(boot
    COMMAND ${LPCXPRESSO_BIN_DIR}/dfu-util -d 0471:df55 -c 0 -t 2048 -R -D ${LPCXPRESSO_BIN_DIR}/LPCXpressoWIN.enc)

endif()


add_custom_target(gdb
  COMMAND ${LPCXPRESSO_GNU_DIR}/arm-none-eabi-gdb --eval-command="set remotetimeout 60000" --eval-command="set arm force-mode thumb"  --eval-command="target extended-remote | ${LPCXPRESSO_BIN_DIR}/crt_emu_cm3_nxp -2 -g -pLPC1769 -vendor=NXP -wire=${LPCXPRESSO_WIRE}" --symbols="${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.axf")

add_custom_target(flash
  COMMAND ${LPCXPRESSO_BIN_DIR}/crt_emu_cm3_nxp -2 -pLPC1769 -vendor=NXP -flash-load-exec="${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.axf" -wire=${LPCXPRESSO_WIRE})