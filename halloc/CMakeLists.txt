project(halloc VERSION 1.0 LANGUAGES CXX)


add_library(halloc INTERFACE)

target_sources(halloc INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/halloc.ipp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/Block.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/halloc.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes.hpp
)

target_include_directories(halloc INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


