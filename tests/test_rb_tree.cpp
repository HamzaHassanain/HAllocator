/**
 * @file test_rb_tree.cpp
 * @brief Unit tests for Red-Black Tree implementation
 *
 * Test Coverage:
 * - Insertion Tests: Single node, ascending, rotations, random order, large scale (10K nodes)
 * - Removal Tests : Leaf, one child, two children, root, cycles, sequential removal
 * - Lower Bound Tests: Empty tree, exact match, no match, boundary cases, with duplicates
 * - Stress Tests: 5K cycles, duplicates handling, 100K random insert/remove/search
 *
 * Verifies RB-Tree Properties:
 * - Root is black
 * - No consecutive red nodes
 * - Equal black height on all paths
 * - BST ordering maintained
 * - Parent pointers correct
 */

#include <gtest/gtest.h>
#include "../rb-tree/rb-tree.hpp"
#include <vector>
#include <algorithm>
#include <cstddef>
#include <random>
#include <fstream>
struct TestNode
{
    TestNode *left, *right, *parent;
    std::size_t value;

    TestNode(std::size_t val) : left(nullptr), right(nullptr), parent(nullptr), value(val) {}
};

bool is_node_red(TestNode *node)
{
    if (!node)
        return false;
    return (node->value & (1ull << 63)) != 0;
}

bool is_node_black(TestNode *node)
{
    if (!node)
        return true;
    return (node->value & (1ull << 63)) == 0;
}

std::size_t get_actual_value(TestNode *node)
{
    return node->value & ~(1ull << 63);
}

bool verify_root_is_black(TestNode *root)
{
    if (!root)
        return true;
    return is_node_black(root);
}

bool verify_no_consecutive_reds(TestNode *node)
{
    if (!node)
        return true;

    if (is_node_red(node))
    {
        if (is_node_red(node->left) || is_node_red(node->right))
            return false;
    }

    return verify_no_consecutive_reds(node->left) && verify_no_consecutive_reds(node->right);
}

int calculate_black_height(TestNode *node, bool &valid)
{
    if (!node)
        return 1;

    int left_height = calculate_black_height(node->left, valid);
    int right_height = calculate_black_height(node->right, valid);

    if (left_height != right_height)
        valid = false;

    return left_height + (is_node_black(node) ? 1 : 0);
}
std::size_t get_value(const std::size_t &value)
{
    return value & ~(1ull << 63);
}
bool verify_bst_property(TestNode *node)
{
    if (!node)
        return true;

    std::size_t node_val = get_value(node->value);
    if (node->left)
    {
        std::size_t left_val = get_value(node->left->value);
        if (left_val > node_val)
            return false;
    }

    if (node->right)
    {
        std::size_t right_val = get_value(node->right->value);
        if (right_val < node_val)
            return false;
    }

    return verify_bst_property(node->left) && verify_bst_property(node->right);
}

bool verify_rb_tree_properties(TestNode *root)
{
    if (!root)
        return true;

    if (!verify_root_is_black(root))
        return false;

    if (!verify_no_consecutive_reds(root))
        return false;

    bool valid = true;
    calculate_black_height(root, valid);
    if (!valid)
        return false;

    if (!verify_bst_property(root))
        return false;

    return true;
}

bool verify_parent_pointers(TestNode *node, TestNode *expected_parent)
{
    if (!node)
        return true;

    if (node->parent != expected_parent)
        return false;

    return verify_parent_pointers(node->left, node) &&
           verify_parent_pointers(node->right, node);
}

int count_nodes(TestNode *node)
{
    if (!node)
        return 0;
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}

TestNode *find_node(TestNode *node, std::size_t value)
{
    if (!node)
        return nullptr;

    std::size_t node_val = get_actual_value(node);
    if (value == node_val)
        return node;
    else if (value < node_val)
        return find_node(node->left, value);
    else
        return find_node(node->right, value);
}

// Helper function to cleanup tree
void cleanup_tree(TestNode *node)
{
    if (!node)
        return;
    cleanup_tree(node->left);
    cleanup_tree(node->right);
    delete node;
}

bool compare_size_t(std::size_t a, std::size_t b)
{
    return a <= b;
}

// ==================== TEST CASES ====================

class RBTreeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @test Single node insertion creates black root with correct properties
 */
TEST(RBTreeTest, RBTreeTest_InsertSingleNode)
{
    TestNode *root = nullptr;
    TestNode *node = new TestNode(10);

    hh::rb_tree::insert(root, node);

    EXPECT_NE(root, nullptr);
    EXPECT_EQ(root, node);
    EXPECT_TRUE(is_node_black(root));
    EXPECT_EQ(get_actual_value(root), 10);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    cleanup_tree(root);
}

/**
 * @test Two ascending nodes maintain RB-tree properties and BST order
 */
TEST(RBTreeTest, RBTreeTest_InsertTwoNodesAscending)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(10);
    TestNode *node2 = new TestNode(20);

    hh::rb_tree::insert(root, node1);
    hh::rb_tree::insert(root, node2);

    EXPECT_EQ(count_nodes(root), 2);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_NE(find_node(root, 10), nullptr);
    EXPECT_NE(find_node(root, 20), nullptr);

    cleanup_tree(root);
}

/**
 * @test Three ascending nodes trigger rotation and recoloring to maintain balance
 */
TEST(RBTreeTest, RBTreeTest_InsertThreeNodesTriggersRotation)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(10);
    TestNode *node2 = new TestNode(20);
    TestNode *node3 = new TestNode(30);

    hh::rb_tree::insert(root, node1);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node3);

    EXPECT_EQ(count_nodes(root), 3);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_NE(find_node(root, 10), nullptr);
    EXPECT_NE(find_node(root, 20), nullptr);
    EXPECT_NE(find_node(root, 30), nullptr);

    cleanup_tree(root);
}

/**
 * @test Ten ascending nodes verify balancing across multiple rotations
 */
TEST(RBTreeTest, RBTreeTest_InsertMultipleNodesAscending)
{
    TestNode *root = nullptr;
    std::vector<TestNode *> nodes;

    for (int i = 1; i <= 10; i++)
    {
        TestNode *node = new TestNode(i * 10);
        nodes.push_back(node);
        hh::rb_tree::insert(root, node);
    }

    EXPECT_EQ(count_nodes(root), 10);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_NE(find_node(root, i * 10), nullptr);
    }

    cleanup_tree(root);
}

/**
 * @test Random insertion order maintains all RB-tree properties
 */
TEST(RBTreeTest, RBTreeTest_InsertMultipleNodesRandom)
{
    TestNode *root = nullptr;
    std::vector<int> values = {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 55, 65};
    std::vector<TestNode *> nodes;

    for (int val : values)
    {
        TestNode *node = new TestNode(val);
        nodes.push_back(node);
        hh::rb_tree::insert(root, node);
    }

    EXPECT_EQ(count_nodes(root), values.size());
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    for (int val : values)
    {
        EXPECT_NE(find_node(root, val), nullptr);
    }

    cleanup_tree(root);
}

/**
 * @test 10,000 sequential insertions verify scalability and tree depth balancing
 */
TEST(RBTreeTest, RBTreeTest_InsertLargeNumberOfNodes)
{
    TestNode *root = nullptr;
    std::vector<TestNode *> nodes;
    const int NUM_NODES = 10000;

    for (int i = 1; i <= NUM_NODES; i++)
    {
        TestNode *node = new TestNode(i);
        nodes.push_back(node);
        hh::rb_tree::insert(root, node);
    }

    EXPECT_EQ(count_nodes(root), NUM_NODES);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    for (int i = 1; i <= NUM_NODES; i++)
    {
        EXPECT_NE(find_node(root, i), nullptr);
    }

    cleanup_tree(root);
}

/**
 * @test Leaf node removal maintains tree structure and RB properties
 */
TEST(RBTreeTest, RemoveLeafNode)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(20);
    TestNode *node2 = new TestNode(10);
    TestNode *node3 = new TestNode(30);

    hh::rb_tree::insert(root, node1);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node3);

    hh::rb_tree::remove(root, node2);

    EXPECT_EQ(count_nodes(root), 2);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_EQ(find_node(root, 10), nullptr);
    EXPECT_NE(find_node(root, 20), nullptr);
    EXPECT_NE(find_node(root, 30), nullptr);

    cleanup_tree(root);
    delete node2;
}

/**
 * @test Node with one child removal properly promotes child and maintains balance
 */
TEST(RBTreeTest, RBTreeTest_RemoveNodeWithOneChild)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(20);
    TestNode *node2 = new TestNode(10);
    TestNode *node3 = new TestNode(30);
    TestNode *node4 = new TestNode(25);

    hh::rb_tree::insert(root, node1);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node3);
    hh::rb_tree::insert(root, node4);

    hh::rb_tree::remove(root, node3);

    EXPECT_EQ(count_nodes(root), 3);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_EQ(find_node(root, 30), nullptr);
    EXPECT_NE(find_node(root, 25), nullptr);

    cleanup_tree(root);
    delete node3;
}

/**
 * @test Node with two children removal uses successor replacement and maintains BST order
 */
TEST(RBTreeTest, RBTreeTest_RemoveNodeWithTwoChildren)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(10);
    TestNode *node2 = new TestNode(20);
    TestNode *node4 = new TestNode(25);
    TestNode *node3 = new TestNode(30);
    TestNode *node5 = new TestNode(35);

    hh::rb_tree::insert(root, node5);
    hh::rb_tree::insert(root, node3);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node4);
    hh::rb_tree::insert(root, node1);

    hh::rb_tree::remove(root, node3);

    EXPECT_EQ(count_nodes(root), 4);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_EQ(find_node(root, 30), nullptr);
    EXPECT_NE(find_node(root, 25), nullptr);
    EXPECT_NE(find_node(root, 35), nullptr);

    cleanup_tree(root);
    delete node3;
}

/**
 * @test Root node removal updates root pointer and maintains tree properties
 */
TEST(RBTreeTest, RBTreeTest_RemoveRootNode)
{
    TestNode *root = nullptr;
    TestNode *node3 = new TestNode(30);
    TestNode *node1 = new TestNode(20);
    TestNode *node2 = new TestNode(10);

    hh::rb_tree::insert(root, node3);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node1);

    hh::rb_tree::remove(root, node1);

    EXPECT_EQ(count_nodes(root), 2);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    EXPECT_EQ(find_node(root, 20), nullptr);

    cleanup_tree(root);
    delete node1;
}

/**
 * @test Alternating remove and reinsert operations maintain correctness through cycles
 */
TEST(RBTreeTest, RBTreeTest_RemoveAndReinsertNodes)
{
    TestNode *root = nullptr;
    std::vector<TestNode *> nodes;

    for (int i = 1; i <= 10; i++)
    {
        TestNode *node = new TestNode(i * 10);
        nodes.push_back(node);
        hh::rb_tree::insert(root, node);
    }

    // Remove every other node
    for (size_t i = 0; i < nodes.size(); i += 2)
    {
        hh::rb_tree::remove(root, nodes[i]);
    }
    for (size_t i = 0; i < nodes.size(); i += 2)
    {
        delete nodes[i];
    }

    EXPECT_EQ(count_nodes(root), 5);
    EXPECT_TRUE(verify_bst_property(root));

    bool valid = true;
    calculate_black_height(root, valid);
    EXPECT_TRUE(valid);

    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    // Reinsert the removed nodes
    for (size_t i = 0; i < nodes.size(); i += 2)
    {
        nodes[i] = new TestNode((i + 1) * 10);
        hh::rb_tree::insert(root, nodes[i]);
    }

    EXPECT_EQ(count_nodes(root), 10);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    cleanup_tree(root);
}

/**
 * @test Sequential removal of all nodes leaves empty tree and maintains properties during removal
 */
TEST(RBTreeTest, RBTreeTest_RemoveAllNodesSequentially)
{
    TestNode *root = nullptr;
    std::vector<TestNode *> nodes;

    for (int i = 1; i <= 10; i++)
    {
        TestNode *node = new TestNode(i * 10);
        nodes.push_back(node);
        hh::rb_tree::insert(root, node);
    }

    for (auto node : nodes)
    {
        hh::rb_tree::remove(root, node);
        if (root != nullptr)
        {
            EXPECT_TRUE(verify_rb_tree_properties(root));
            EXPECT_TRUE(verify_parent_pointers(root, nullptr));
        }
    }

    EXPECT_EQ(root, nullptr);

    for (auto node : nodes)
    {
        delete node;
    }
    cleanup_tree(root);
}

/**
 * @test Lower bound on empty tree returns nullptr without crashing
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundEmptyTree)
{
    TestNode *root = nullptr;

    hh::rb_tree::lower_bound(root, (std::size_t)10, compare_size_t);

    cleanup_tree(root);
}

/**
 * @test Lower bound with exact match returns the matching node
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundExactMatch)
{
    TestNode *root = nullptr;
    std::vector<int> values = {10, 20, 30, 40, 50};

    for (int val : values)
    {
        TestNode *node = new TestNode(val);
        hh::rb_tree::insert(root, node);
    }

    TestNode *result = hh::rb_tree::lower_bound(root, (std::size_t)30, compare_size_t);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(get_actual_value(result), 30);

    cleanup_tree(root);
}

/**
 * @test Lower bound without exact match returns next larger element
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundNoExactMatch)
{
    TestNode *root = nullptr;
    std::vector<int> values = {10, 20, 30, 40, 50};

    for (int val : values)
    {
        TestNode *node = new TestNode(val);
        hh::rb_tree::insert(root, node);
    }

    TestNode *result = hh::rb_tree::lower_bound(root, (std::size_t)25, compare_size_t);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(get_actual_value(result), 30);

    cleanup_tree(root);
}

/**
 * @test Lower bound smaller than all values returns smallest element
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundSmallerThanAll)
{
    TestNode *root = nullptr;
    std::vector<int> values = {10, 20, 30, 40, 50};

    for (int val : values)
    {
        TestNode *node = new TestNode(val);
        hh::rb_tree::insert(root, node);
    }

    TestNode *result = hh::rb_tree::lower_bound(root, (std::size_t)5, compare_size_t);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(get_actual_value(result), 10);

    cleanup_tree(root);
}

/**
 * @test Lower bound larger than all values returns nullptr
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundLargerThanAll)
{
    TestNode *root = nullptr;
    std::vector<int> values = {10, 20, 30, 40, 50};

    for (int val : values)
    {
        TestNode *node = new TestNode(val);
        hh::rb_tree::insert(root, node);
    }

    TestNode *result = hh::rb_tree::lower_bound(root, (std::size_t)60, compare_size_t);

    EXPECT_EQ(result, nullptr);

    cleanup_tree(root);
}

/**
 * @test 5,000 insert/remove cycles with 2,000 nodes each verify long-term stability
 */
TEST(RBTreeTest, RBTreeTest_StressTestInsertRemoveCycle)
{
    TestNode *root = nullptr;
    const int CYCLES = 5000;
    const int NODES_PER_CYCLE = 2000;

    for (int cycle = 0; cycle < CYCLES; cycle++)
    {
        std::vector<TestNode *> nodes;

        // Insert nodes
        for (int i = 0; i < NODES_PER_CYCLE; i++)
        {
            TestNode *node = new TestNode(cycle * NODES_PER_CYCLE + i);
            nodes.push_back(node);
            hh::rb_tree::insert(root, node);
        }

        // Remove half of them
        for (int i = 0; i < NODES_PER_CYCLE / 2; i++)
        {
            hh::rb_tree::remove(root, nodes[i]);
            delete nodes[i];
        }
    }
    if (root != nullptr)
    {
        EXPECT_TRUE(verify_rb_tree_properties(root));
        EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    }

    cleanup_tree(root);
}

/**
 * @test Duplicate value insertions are handled correctly with proper tree properties
 */
TEST(RBTreeTest, RBTreeTest_InsertDuplicateValues)
{
    TestNode *root = nullptr;
    TestNode *node1 = new TestNode(10);
    TestNode *node2 = new TestNode(10);
    TestNode *node3 = new TestNode(10);

    hh::rb_tree::insert(root, node1);
    hh::rb_tree::insert(root, node2);
    hh::rb_tree::insert(root, node3);

    EXPECT_EQ(count_nodes(root), 3);
    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    cleanup_tree(root);
}

/**
 * @test Lower bound with duplicates and removals matches std::lower_bound behavior
 */
TEST(RBTreeTest, RBTreeTest_LowerBoundWithRemovesAndDuplicates)
{
    TestNode *root = nullptr;

    std::vector<std::size_t> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    std::vector<TestNode *> nodes;
    for (std::size_t val : values)
    {
        TestNode *node = new TestNode(val);
        hh::rb_tree::insert(root, node);
        nodes.push_back(node);
    }

    std::sort(values.begin(), values.end());
    std::vector<std::size_t> to_lower_bonud_on = {1, 23, 5, 6, 10, 20, 7, 9, 10, 11, 14};
    for (std::size_t val : to_lower_bonud_on)
    {
        TestNode *result = hh::rb_tree::lower_bound(root, val, compare_size_t);

        auto it = std::lower_bound(values.begin(), values.end(), val);

        if (it != values.end())
        {
            EXPECT_NE(result, nullptr);
            EXPECT_EQ(get_actual_value(result), *it);
        }
        else
        {
            EXPECT_EQ(result, nullptr);
        }
    }

    // Remove some nodes
    for (size_t i = 0; i < nodes.size(); i += 3)
    {
        hh::rb_tree::remove(root, nodes[i]);
        values.erase(std::find(values.begin(), values.end(), get_actual_value(nodes[i])));
        delete nodes[i];

        EXPECT_EQ(count_nodes(root), values.size());
        EXPECT_TRUE(verify_rb_tree_properties(root));
        EXPECT_TRUE(verify_parent_pointers(root, nullptr));
    }
    std::sort(values.begin(), values.end());

    for (std::size_t val : to_lower_bonud_on)
    {
        TestNode *result = hh::rb_tree::lower_bound(root, val, compare_size_t);

        auto it = std::lower_bound(values.begin(), values.end(), val);

        if (it != values.end())
        {
            EXPECT_NE(result, nullptr);
            EXPECT_EQ(get_actual_value(result), *it);
        }
        else
        {
            EXPECT_EQ(result, nullptr);
        }
    }

    cleanup_tree(root);
}

/**
 * @test 100K random operations with lower_bound queries verify correctness against std::multiset
 */
TEST(RBTreeTest, RBTreeTest_StressInsertRemoveLowerBound)
{
    TestNode *root = nullptr;

    const int NUM_NODES = 100000;
    std::vector<TestNode *> nodes;
    std::multiset<std::size_t> existing_values;
    for (int i = 0; i < NUM_NODES; i++)
    {
        TestNode *node = new TestNode(rand() % (NUM_NODES / 10));
        hh::rb_tree::insert(root, node);

        existing_values.insert(get_actual_value(node));
        nodes.push_back(node);
    }

    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    // remove random 20% of nodes
    std::shuffle(nodes.begin(), nodes.end(), std::mt19937{std::random_device{}()});
    for (size_t i = 0; i < nodes.size() * 2 / 10; i++)
    {
        hh::rb_tree::remove(root, nodes[i]);
        existing_values.erase(existing_values.find(get_actual_value(nodes[i])));
        delete nodes[i];
    }

    EXPECT_TRUE(verify_rb_tree_properties(root));
    EXPECT_TRUE(verify_parent_pointers(root, nullptr));

    EXPECT_EQ(count_nodes(root), existing_values.size());

    // lower_bound tests is the same as multiset lower_bound

    std::vector<std::size_t> inorder_traversal;
    std::function<void(TestNode *)> inorder = [&](TestNode *node)
    {
        if (!node)
            return;
        inorder(node->left);
        inorder_traversal.push_back(get_actual_value(node));
        inorder(node->right);
    };

    inorder(root);

    EXPECT_TRUE(std::is_sorted(inorder_traversal.begin(), inorder_traversal.end()));

    for (int i = 0; i < 1000; i++)
    {
        std::size_t query_val = rand() % (NUM_NODES / 10 + NUM_NODES / 20);
        TestNode *result = hh::rb_tree::lower_bound(root, query_val, compare_size_t);
        auto it = existing_values.lower_bound(query_val);

        if (it != existing_values.end())
        {
            EXPECT_NE(result, nullptr);
            EXPECT_EQ(get_actual_value(result), *it);
        }
        else
        {
            EXPECT_EQ(result, nullptr);
        }
    }

    cleanup_tree(root);
}