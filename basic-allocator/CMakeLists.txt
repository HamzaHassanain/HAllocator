

project(basic_alloc VERSION 1.0 LANGUAGES CXX)

add_library(basic_alloc STATIC 
    hh_basic_alloc.cpp
    hh_basic_alloc.hpp
)


target_include_directories(basic_alloc PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

