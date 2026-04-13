cmake_minimum_required(VERSION 3.10)
project(TaskServer LANGUAGES CXX)

# Стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск pthread
find_package(Threads REQUIRED)

# Создаём исполняемый файл
add_executable(task_server main.cpp)

# Линкуем pthread
target_link_libraries(task_server PRIVATE Threads::Threads)

# Флаги компиляции
target_compile_options(task_server PRIVATE -Wall -Wextra -O2)

