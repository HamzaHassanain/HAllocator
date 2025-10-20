project(halloc VERSION 1.0 LANGUAGES CXX)


add_library(halloc STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/Block.cpp)

target_sources(halloc INTERFACE

  ${CMAKE_CURRENT_SOURCE_DIR}/includes/Block.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/BlocksContainer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/includes/Halloc.hpp
)

target_include_directories(halloc INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


