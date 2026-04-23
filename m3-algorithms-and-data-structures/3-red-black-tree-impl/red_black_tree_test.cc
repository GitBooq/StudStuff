#include "red_black_tree.h"
#include <gtest/gtest.h>
#include <sstream>
#include <string_view>

namespace RBTree {
template class RedBlackTree<int>;
template void swap(RedBlackTree<int, std::less<>> &,
                   RedBlackTree<int, std::less<>> &);
static_assert(std::bidirectional_iterator<RedBlackTree<int>::Iterator>);
} // namespace RBTree

TEST(RBTreeTest, SimpleTest) {
  RBTree::RedBlackTree<int> tree;

  EXPECT_EQ(tree.size(), 0);

  tree.Insert(1337);
  tree.Insert(1337);
  tree.Insert(1337);
  tree.Insert(7);
  tree.Insert(3);
  tree.Insert(1);
  tree.Insert(3);

  EXPECT_EQ(tree.size(), 7);

  auto node_exist = tree.Find(1337);
  auto node_not_exist = tree.Find(2);

  EXPECT_NE(node_exist, tree.end());
  EXPECT_EQ(node_not_exist, tree.end());

  auto removed_count = tree.Remove(*node_exist);
  EXPECT_EQ(removed_count, 3);

  std::string_view expected{"1 3 3 7 "};
  std::stringstream actual;
  tree.Print(actual);

  EXPECT_EQ(expected, actual.view());

  EXPECT_FALSE(tree.empty());

  tree.Clear();
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeTest, SwapTest) {
  RBTree::RedBlackTree<int> tree_1;
  RBTree::RedBlackTree<int> tree_2;

  tree_1.Insert(2);
  tree_1.Insert(3);
  tree_1.Insert(1);

  tree_2.Insert(6);
  tree_2.Insert(4);
  tree_2.Insert(5);

  std::string_view tree_1_expected{"4 5 6 "};
  std::string_view tree_2_expected{"1 2 3 "};
  std::stringstream tree_1_actual;
  std::stringstream tree_2_actual;

  std::swap(tree_1, tree_2);

  tree_1.Print(tree_1_actual);
  tree_2.Print(tree_2_actual);

  EXPECT_EQ(tree_1_actual.view(), tree_1_expected);
  EXPECT_EQ(tree_2_actual.view(), tree_2_expected);
}

TEST(RBTreeTest, CopyTest) {
  RBTree::RedBlackTree<int> tree_1;

  tree_1.Insert(2);
  tree_1.Insert(3);
  tree_1.Insert(1);

  RBTree::RedBlackTree<int> tree_2(tree_1);

  std::string_view tree_1_expected{"1 2 3 "};
  std::stringstream tree_1_actual;
  std::stringstream tree_2_actual;

  tree_1.Print(tree_1_actual);
  tree_2.Print(tree_2_actual);

  EXPECT_EQ(tree_1_actual.view(), tree_1_expected);
  EXPECT_EQ(tree_2_actual.view(), tree_1_expected);

  tree_2.Clear();
  EXPECT_EQ(tree_1_actual.view(), tree_1_expected);
}