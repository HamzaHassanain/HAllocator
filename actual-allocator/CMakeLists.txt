project(actual_alloc VERSION 1.0 LANGUAGES CXX)


add_library(actual_alloc INTERFACE)

target_sources(actual_alloc INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/halloc.ipp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/Block.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/halloc.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/alloc.hpp
)

target_include_directories(actual_alloc INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

