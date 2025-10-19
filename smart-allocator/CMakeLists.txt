

project(smart_alloc VERSION 1.0 LANGUAGES CXX)

add_library(smart_alloc STATIC 
    hh_smart_alloc.cpp
    hh_smart_alloc.hpp
)


target_include_directories(smart_alloc PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

