project(rb_tree VERSION 1.0 LANGUAGES CXX)


add_library(rb_tree INTERFACE)

target_sources(rb_tree INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/rb-tree.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rb-tree.ipp
)

target_include_directories(rb_tree INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

