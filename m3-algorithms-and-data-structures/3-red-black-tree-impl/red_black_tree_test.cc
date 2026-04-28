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

namespace test {
class RBTreeChecker {
public:
  template <typename Key, typename Compare>
  static bool Validate(const RBTree::RedBlackTree<Key, Compare> &tree) {
    if (!IsInorder(tree)) {
      return false;
    }
    if (!NoDoubleRed(tree)) {
      return false;
    }
    if (!SameBlackHeight(tree)) {
      return false;
    }
    if (!RootIsBlack(tree)) {
      return false;
    }
    return true;
  }

  // Checks BST property using iterator-based in-order traversal
  template <typename Key, typename Compare>
  static bool IsInorder(const RBTree::RedBlackTree<Key, Compare> &tree) {
    return std::is_sorted(tree.begin(), tree.end(), tree.comp_);
  }

  // Checks RBTree property #4: If node is Red then both children are Black
  template <typename Key, typename Compare>
  static bool NoDoubleRed(const RBTree::RedBlackTree<Key, Compare> &tree) {
    if (tree.root_ == nullptr) {
      return true;
    }

    using Node = RBTree::RedBlackTree<Key, Compare>::NodeType;
    std::vector<Node *> stack = {tree.root_};

    while (!stack.empty()) {
      Node *node = stack.back();
      stack.pop_back();

      if (node->color == RBTree::Color::kRed) {
        if (RBTree::RedBlackTree<Key, Compare>::ColorOf(node->left) ==
                RBTree::Color::kRed ||
            RBTree::RedBlackTree<Key, Compare>::ColorOf(node->right) ==
                RBTree::Color::kRed) {
          return false;
        }
      }

      if (node->left) {
        stack.push_back(node->left);
      }
      if (node->right) {
        stack.push_back(node->right);
      }
    }

    return true;
  }

  // Checks RBTree property #5: For each node every pathes from it to leafs
  // that are it descendents contain same amount of black nodes
  template <typename Key, typename Compare>
  static bool SameBlackHeight(const RBTree::RedBlackTree<Key, Compare> &tree) {
    if (tree.root_ == nullptr) {
      return true;
    }

    using Node = RBTree::RedBlackTree<Key, Compare>::NodeType;

    struct StackFrame {
      Node *node;       // current node
      int black_height; // black height from root to this node
    };

    std::vector<StackFrame> stack;
    stack.push_back(
        {tree.root_, tree.root_->color == RBTree::Color::kBlack ? 1 : 0});

    // expected_black_height will be updated w/ first NIL encounter
    std::optional<int> expected_black_height = std::nullopt;

    // DFS
    while (!stack.empty()) {
      auto [node, current_black_height] = stack.back();
      stack.pop_back();

      auto process_child = [&](Node *child) {
        if (child != nullptr) {
          int child_black_height =
              current_black_height +
              (child->color == RBTree::Color::kBlack ? 1 : 0);
          stack.push_back({child, child_black_height});
        } else {
          // NIL
          int nil_black = current_black_height + 1;
          if (!expected_black_height) {
            expected_black_height = nil_black;
          } else if (expected_black_height != nil_black) {
            return false;
          }
        }
        return true;
      };

      if (!process_child(node->left)) {
        return false;
      }
      if (!process_child(node->right)) {
        return false;
      }
    }

    return true;
  }

  // Checks RBTree property #2: Root is Black
  template <typename Key, typename Compare>
  static bool RootIsBlack(const RBTree::RedBlackTree<Key, Compare> &tree) {
    return RBTree::RedBlackTree<int>::ColorOf(tree.root_) ==
           RBTree::Color::kBlack;
  }
};
} // namespace test

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

class RBTreeTestF : public ::testing::Test {
protected:
  void SetUp() override {
    for (int i = 0; i < kTreeSize; ++i) {
      tree.Insert(i);
    }
  }

  static const int kTreeSize{1000};
  RBTree::RedBlackTree<int> tree; // uniq nodes
};

TEST_F(RBTreeTestF, OrderedTree) {
  auto isValidBST = test::RBTreeChecker::IsInorder(tree);

  EXPECT_TRUE(isValidBST);
}

TEST_F(RBTreeTestF, RootIsBlack) {
  auto root_is_black = test::RBTreeChecker::RootIsBlack(tree);

  EXPECT_TRUE(root_is_black);
}

TEST_F(RBTreeTestF, NoDoubleRed) {
  auto no_double_red = test::RBTreeChecker::NoDoubleRed(tree);

  EXPECT_TRUE(no_double_red);
}

TEST_F(RBTreeTestF, SameBlackHeight) {
  auto same_blck_height = test::RBTreeChecker::SameBlackHeight(tree);

  EXPECT_TRUE(same_blck_height);
}

TEST_F(RBTreeTestF, ValidRBTreeAfterOneNodeDelete) {
  auto key = *tree.begin();

  tree.Remove(key);

  EXPECT_TRUE(test::RBTreeChecker::Validate(tree));
}

TEST_F(RBTreeTestF, ValidRBTreeUniqAfterAllNodesDelete) {
  for (auto it = tree.begin(), end = tree.end(); it != end;) {
    auto key = *it;

    ++it;

    tree.Remove(key);

    EXPECT_TRUE(test::RBTreeChecker::Validate(tree));
  }
}

TEST_F(RBTreeTestF, ValidRBTreeWithDuplicatesAfterAllNodesDelete) {
  RBTree::RedBlackTree<int> dup_tree;
  const int kTreeSize = 500;
  for (int i = 0; i < kTreeSize; ++i) {
    dup_tree.Insert(i);
    dup_tree.Insert(i);
  }

  // delete keys by one using iterators
  while (!dup_tree.empty()) {
    auto iter = dup_tree.begin();
    iter = dup_tree.Remove(iter);

    EXPECT_TRUE(test::RBTreeChecker::Validate(dup_tree));
  }
}

TEST_F(RBTreeTestF, ValidRBTreeAfterClear) {
  tree.Clear();
  EXPECT_TRUE(test::RBTreeChecker::Validate(tree));
}

TEST(RBTreeTest, ValidEmptyRBTree) {
  RBTree::RedBlackTree<int> cont;

  EXPECT_TRUE(test::RBTreeChecker::Validate(cont));
}

TEST(RBTreeTest, ValidRBTreeAfterNodeInsert) {
  RBTree::RedBlackTree<int> cont;

  const int kElementsToInsert{1000};
  for (int i = 0; i < kElementsToInsert; ++i) {
    cont.Insert(i);

    EXPECT_TRUE(test::RBTreeChecker::Validate(cont));
  }
}
