cmake_minimum_required(VERSION 3.10)
project(HeatSolverOpenACC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS program_options)

# Флаги оптимизации
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fast -Minfo=all")

# Режим компиляции
if(NOT DEFINED ACC_MODE)
    set(ACC_MODE "gpu" CACHE STRING "ACC target: host, multicore, gpu")
endif()

if(ACC_MODE STREQUAL "gpu")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=gpu")
elseif(ACC_MODE STREQUAL "multicore")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=multicore")
elseif(ACC_MODE STREQUAL "host")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc=host")
endif()

# Исходные файлы
set(SOURCES main.cpp heat_solver.cpp)

add_executable(heat_solver ${SOURCES})

target_include_directories(heat_solver PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(heat_solver PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARY})

message(STATUS "Building HeatSolver with ACC_MODE=${ACC_MODE}")
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS}")