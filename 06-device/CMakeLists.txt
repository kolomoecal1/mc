cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(06_device)

add_subdirectory("../libs/protocol" "${PROJECT_BINARY_DIR}/protocol")
add_subdirectory("../libs/bme280-driver" "${PROJECT_BINARY_DIR}/bme280_driver")
add_subdirectory("../libs/ili9341-driver" "${PROJECT_BINARY_DIR}/ili9341_driver")

pico_sdk_init()

add_executable(${PROJECT_NAME}
    main.c
    gui/gui_screen.c
    gui/gui_graph.c
    sensors/bme280_wrapper.c
    display/display_wrapper.c
    tasks/measurement_task.c
    tasks/ui_task.c
)

pico_set_linker_script(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_i2c
    protocol
    bme280_driver
    ili9341_driver
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/sensors
    ${CMAKE_CURRENT_SOURCE_DIR}/display
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks
)

pico_add_extra_outputs(${PROJECT_NAME})