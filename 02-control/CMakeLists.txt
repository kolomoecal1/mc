cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(02_control)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol-task/protocol-task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/led-task/led-task.c
)

pico_set_linker_script(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio  
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol-task
    ${CMAKE_CURRENT_SOURCE_DIR}/led-task 
)

pico_add_extra_outputs(${PROJECT_NAME})