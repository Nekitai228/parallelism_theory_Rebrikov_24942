cmake_minimum_required(VERSION 3.10)
project(TaskServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Основная программа (сервер + клиенты)
add_executable(task_server 
    main.cpp
)
target_link_libraries(task_server PRIVATE Threads::Threads)

# Отдельная программа для валидации результатов
add_executable(validator 
    validator.cpp
)
# validator не требует pthread, но можно добавить для консистентности
target_link_libraries(validator PRIVATE Threads::Threads)

# Флаги компиляции
target_compile_options(task_server PRIVATE -Wall -Wextra -O2)
target_compile_options(validator PRIVATE -Wall -Wextra -O2)