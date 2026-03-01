# CMakeLists.txt для integral.cpp
cmake_minimum_required(VERSION 3.10)
project(integral LANGUAGES CXX)

# Стандарт C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск OpenMP
find_package(OpenMP REQUIRED)

# Исполняемый файл
add_executable(integral integral.cpp)

# Подключаем OpenMP
target_link_libraries(integral PRIVATE OpenMP::OpenMP_CXX)

# Оптимизации для Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(integral PRIVATE -O3 -march=native)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(integral PRIVATE /O2)
    endif()
endif()