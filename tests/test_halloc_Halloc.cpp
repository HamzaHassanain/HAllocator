/**
 * @file test_halloc_Halloc.cpp
 * @brief Unit tests for Halloc, with STL container integration
 *
 * @TODO Add more tests for different STL containers (map, list, etc.)
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

#include "../halloc/includes/Halloc.hpp"

using namespace hh::halloc;

class HallocTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST(HallocTest, STRESS_TestWithVector) {
    // Test that Halloc works with std::vector

    std::vector<int, Halloc<int, 1024 * 1024 * 256>> v;

    const int COUNT = (1024 * 1024 * 16);
    // v.reserve(COUNT); // Reserve space for 1M elements
    for (int i = 0; i < COUNT; i++) {
        v.push_back(i);
    }
    // Verify all values
    for (int i = 0; i < COUNT; i++) {
        EXPECT_EQ(v[i], i);
    }

    EXPECT_EQ(v.size(), COUNT);
}

TEST(HallocTest, STRESS_TestWithVectorOfStructs) {
    struct MyStruct {
        int id;
        char data[64];
        double value;
    };

    std::vector<MyStruct, Halloc<MyStruct, 1024 * 1024 * 256>> v;

    const int COUNT = (1024 * 1024);
    for (int i = 0; i < COUNT; i++) {
        MyStruct ms;
        ms.id = i;
        ms.value = i * 1.5;
        std::strcpy(ms.data, ("Struct_" + std::to_string(i)).c_str());
        v.push_back(ms);
    }
    // Verify all values
    for (int i = 0; i < COUNT; i++) {
        EXPECT_EQ(v[i].id, i);
        EXPECT_STREQ(v[i].data, ("Struct_" + std::to_string(i)).c_str());
        EXPECT_DOUBLE_EQ(v[i].value, i * 1.5);
    }
}
TEST(HallocTest, STRESS_TestWithSet) {
    std::set<int, std::less<int>, Halloc<int, 1024 * 1024 * 256>> s;
    const int COUNT = (1024 * 1024);
    for (int i = 0; i < COUNT; i++) {
        s.insert(i);
    }

    // Verify all values
    for (int i = 0; i < COUNT; i++) {
        EXPECT_TRUE(s.find(i) != s.end());
    }

    // erase all numbers div by 4
    for (int i = 0; i < COUNT; i += 4) {
        s.erase(i);
    }

    // insert random numbers
    std::mt19937 rng(std::random_device{}());
    for (int i = 0; i < COUNT / 4; i++) {
        int num = rng() % COUNT;
        s.insert(num);
    }
}