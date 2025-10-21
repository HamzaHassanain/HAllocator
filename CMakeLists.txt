cmake_minimum_required(VERSION 3.10)
project(hallocator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For clang-tidy

# Default build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g -O0")

# ==================== SANITIZERS ====================
# Usage: cmake -DSANITIZER=address ..
#        cmake -DSANITIZER=thread ..
#        cmake -DSANITIZER=undefined ..
#        cmake -DSANITIZER=memory ..

set(SANITIZER "" CACHE STRING "Sanitizer to enable (address, thread, undefined, memory, leak)")

if(SANITIZER)
    message(STATUS "Enabling ${SANITIZER} sanitizer")
    
    if(SANITIZER STREQUAL "address")
        set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
    elseif(SANITIZER STREQUAL "thread")
        set(SANITIZER_FLAGS "-fsanitize=thread")
    elseif(SANITIZER STREQUAL "undefined")
        set(SANITIZER_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all")
    elseif(SANITIZER STREQUAL "memory")
        set(SANITIZER_FLAGS "-fsanitize=memory -fno-omit-frame-pointer")
    elseif(SANITIZER STREQUAL "leak")
        set(SANITIZER_FLAGS "-fsanitize=leak")
    else()
        message(FATAL_ERROR "Unknown sanitizer: ${SANITIZER}")
    endif()
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

# ==================== CLANG-TIDY ====================
# Enable with: cmake -DENABLE_CLANG_TIDY=ON ..

option(ENABLE_CLANG_TIDY "Enable clang-tidy checks" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*")
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()


# ==================== SUBDIRECTORIES ====================

add_subdirectory(basic-allocator)
add_subdirectory(rb-tree)
add_subdirectory(halloc)


# ===================== Build Library =====================
add_library(hallocator STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/halloc/src/Block.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basic-allocator/basic_alloc.cpp
)

target_link_libraries( hallocator PUBLIC 
    basic_alloc
    rb_tree
    halloc)


# ==================== DOXYGEN ====================

option(BUILD_DOC "Build documentation" OFF)

if(BUILD_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        message(STATUS "Doxygen found. Use 'make doc' or 'cmake --build . --target doc' to generate documentation.")
    else()
        message(WARNING "Doxygen not found. Documentation will not be generated.")
    endif()
endif()


# ==================== TESTING ====================

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_subdirectory(tests)



