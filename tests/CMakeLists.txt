cmake_minimum_required(VERSION 3.10)

# Add test executables
add_executable(allocator_tests
    test_basic_alloc.cpp
)

# Link against gtest
target_link_libraries(allocator_tests
    
    basic_alloc
    GTest::gtest_main
)

# Include directories
target_include_directories(allocator_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/basic-allocator
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(allocator_tests)
