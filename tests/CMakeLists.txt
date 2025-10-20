cmake_minimum_required(VERSION 3.10)

# Add test executables
add_executable(allocator_tests
    test_basic_alloc.cpp
    test_rb_tree.cpp
    test_halloc_Block.cpp
    test_halloc_BlocksContainer.cpp
    test_halloc_Halloc.cpp
)

# Link against gtest
target_link_libraries(allocator_tests
    
    basic_alloc
    halloc
    GTest::gtest_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(allocator_tests)
