cmake_minimum_required(VERSION 3.19)
project(c C)

set(CMAKE_C_STANDARD 11)

add_executable(c main.c pcre_result_array_list.c)
add_executable(demo demo.c)

target_link_libraries(c pcre2-8)
target_link_libraries(demo pthread)