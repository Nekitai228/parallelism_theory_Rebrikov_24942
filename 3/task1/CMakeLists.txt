cmake_minimum_required(VERSION 3.10)
project(MatrixVectorProduct LANGUAGES CXX)

# C++20 стандарт
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск pthread
find_package(Threads REQUIRED)

# Создаём исполняемый файл
add_executable(dgemv dgemv.cpp)

# Линкуем pthread
target_link_libraries(dgemv PRIVATE Threads::Threads)

# Оптимизации
target_compile_options(dgemv PRIVATE -O3 -march=native -Wall -Wextra)