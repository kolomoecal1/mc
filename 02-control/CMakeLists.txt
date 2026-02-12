# Требуем минимальную версию cmake
cmake_minimum_required(VERSION 3.15)

# Задаем параметры платы
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

# Подключаем Pico SDK
include(pico_sdk_import.cmake)

# Создаем проект - ИМЯ МЕНЯЕТСЯ ТОЛЬКО ЗДЕСЬ!
project(02_control)

# Инициализация SDK
pico_sdk_init()

# Добавляем файлы исходного кода
add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c   # <-- ПРАВИЛЬНЫЙ ПУТЬ
)

# Указываем linker script
pico_set_linker_script(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)

# Настройка USB вместо UART
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Подключаем библиотеки
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
)

# Добавляем директории с заголовочными файлами
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task
)

# Генерируем дополнительные выходные файлы
pico_add_extra_outputs(${PROJECT_NAME})