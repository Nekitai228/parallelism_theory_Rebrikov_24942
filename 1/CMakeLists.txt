cmake_minimum_required(VERSION 3.10)
project(Task1)

# 1. Настраиваем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Опция для выбора float/double (если нужно через CMake)
option(USE_DOUBLE "Use double precision instead of float" OFF)

# 3. Настраиваем конфигурационный файл (если config.h.in существует)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in")
    configure_file(
        config.h.in
        ${CMAKE_BINARY_DIR}/config.h
    )
    include_directories(${CMAKE_BINARY_DIR})
endif()


# 4. Добавляем исполняемый файл
add_executable(task1 task1.cpp)


# 5. Если есть папка build - игнорируем её
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT task1)