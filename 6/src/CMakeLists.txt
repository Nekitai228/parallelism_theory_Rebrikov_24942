cmake_minimum_required(VERSION 3.10)
project(HeatSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER "pgc++" CACHE STRING "C++ compiler")

find_package(Boost REQUIRED COMPONENTS program_options)

set(ACC_MODE "host" CACHE STRING "OpenACC target")
set_property(CACHE ACC_MODE PROPERTY STRINGS "host" "multicore" "gpu")

# Агрессивная оптимизация + вывод распараллеливания
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Minfo=all -fast")

if(ACC_MODE STREQUAL "host")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=host")
elseif(ACC_MODE STREQUAL "multicore")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=multicore")
elseif(ACC_MODE STREQUAL "gpu")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=gpu")
endif()

add_executable(heat_solver main.cpp heat_solver.cpp)
target_link_libraries(heat_solver PRIVATE Boost::program_options)