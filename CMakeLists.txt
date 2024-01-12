cmake_minimum_required (VERSION 3.22)

project(LAB4)

find_package(GDAL REQUIRED)
find_package(OpenMP REQUIRED)

file(MAKE_DIRECTORY bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE src/main.c src/processes.c src/strips.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_CXX_FLAGS} -g -Wall -Werror -pedantic -Wextra -Wconversion -std=gnu11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(lab4 ${SOURCE})

target_include_directories(lab4 PRIVATE ${GDAL_INCLUDE_DIRS})

target_link_libraries(lab4 ${GDAL_LIBRARIES})
target_link_libraries(lab4 ${OpenMP_CXX_FLAGS})