# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(dgemv LANGUAGES CXX)

# Стандарт C++ (11 достаточно для chrono, vector, etc.)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск OpenMP
find_package(OpenMP REQUIRED)

# Создаём исполняемый файл
add_executable(dgemv dgemv.cpp)

# Подключаем OpenMP к целевому файлу
target_link_libraries(dgemv PRIVATE OpenMP::OpenMP_CXX)

# Оптимизации для Release-сборки
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(dgemv PRIVATE -O3 -march=native)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(dgemv PRIVATE /O2)
    endif()
endif()