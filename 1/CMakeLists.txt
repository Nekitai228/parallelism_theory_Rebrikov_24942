cmake_minimum_required(VERSION 3.10)
project(SinCalculator)

# Опция для выбора типа данных при сборке
option(USE_DOUBLE "Build with double precision" OFF)

# Создаем две версии программы
if(USE_DOUBLE)
    add_executable(sin_calc_double task1.cpp)
    target_compile_options(sin_calc_double PRIVATE -O3)
    # Будем запускать с аргументом "double"
else()
    add_executable(sin_calc_float task1.cpp)
    target_compile_options(sin_calc_float PRIVATE -O3)
    # Будем запускать с аргументом "float"
endif()

# Или универсальная версия
add_executable(sin_calc task1.cpp)
target_compile_options(sin_calc PRIVATE -O3)