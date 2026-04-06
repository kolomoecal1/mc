cmake_minimum_required(VERSION 3.15)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(05_display)
pico_sdk_init()
add_subdirectory("C:/Repositories/pico/mcu/libs/protocol" "${PROJECT_BINARY_DIR}/protocol")
add_subdirectory("C:/Repositories/pico/mcu/libs/ili9341-driver" "${PROJECT_BINARY_DIR}/ili9341_driver")

add_executable(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c
	${CMAKE_CURRENT_SOURCE_DIR}/led-task/led-task.c
)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    protocol
    hardware_spi
    hardware_gpio
    ili9341_driver
)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)
pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)