#include "red_black_tree.h"
#include <format>
#include <gtest/gtest.h>
#include <random>
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
    if (tree.root_ == tree.nil_) {
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

      if (node->left != tree.nil_) {
        stack.push_back(node->left);
      }
      if (node->right != tree.nil_) {
        stack.push_back(node->right);
      }
    }

    return true;
  }

  // Checks RBTree property #5: For each node every pathes from it to leafs
  // that are it descendents contain same amount of black nodes
  template <typename Key, typename Compare>
  static bool SameBlackHeight(const RBTree::RedBlackTree<Key, Compare> &tree) {
    if (tree.root_ == tree.nil_) {
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

    std::optional<int> expected_black_height = std::nullopt;

    while (!stack.empty()) {
      auto [node, current_black] = stack.back();
      stack.pop_back();

      if (node->left == tree.nil_ && node->right == tree.nil_) {
        if (!expected_black_height) {
          expected_black_height = current_black;
        } else if (expected_black_height != current_black) {
          std::cout << "Black height differ!\n";
          return false;
        }
        continue;
      }

      if (node->left != tree.nil_) {
        int left_black = current_black +
                         (node->left->color == RBTree::Color::kBlack ? 1 : 0);
        stack.push_back({node->left, left_black});
      }
      if (node->right != tree.nil_) {
        int right_black = current_black +
                          (node->right->color == RBTree::Color::kBlack ? 1 : 0);
        stack.push_back({node->right, right_black});
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
  tree.Insert(7);
  tree.Insert(1);
  tree.Insert(3);

  EXPECT_EQ(tree.size(), 4);

  auto node_exist = tree.Find(1337);
  auto node_not_exist = tree.Find(2);

  EXPECT_NE(node_exist, tree.end());
  EXPECT_EQ(node_not_exist, tree.end());

  auto removed_count = tree.Remove(*node_exist);
  EXPECT_EQ(removed_count, 1);

  std::string_view expected{"1 3 7 "};
  std::stringstream actual;
  tree.Print(actual);

  EXPECT_EQ(expected, actual.view());

  EXPECT_FALSE(tree.empty());

  tree.Clear();
  EXPECT_TRUE(tree.empty());
}

TEST(RBTreeTest, PrintTest) {
  RBTree::RedBlackTree<int> tree;

  tree.Insert(7);
  tree.Insert(1);
  tree.Insert(3);

  EXPECT_EQ(tree.size(), 3);

  std::string_view expected{"1 3 7 "};
  std::stringstream actual;
  tree.Print(actual);

  EXPECT_EQ(expected, actual.view());
}

TEST(RBTreeTest, DISABLED_SimpleTestDuplicates) {
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

TEST(RBTreeTest, LowerBoundTest) {
  RBTree::RedBlackTree<int> tree;
  tree.Insert(1);
  auto first = tree.Insert(2);
  tree.Insert(2);

  auto result_exist = tree.LowerBound(2);

  EXPECT_EQ(result_exist, first);

  auto result_not_exist = tree.LowerBound(3);

  EXPECT_EQ(result_not_exist, tree.end());
}

TEST(RBTreeTest, UpperBoundTest) {
  RBTree::RedBlackTree<int> tree;
  tree.Insert(1);
  auto first = tree.Insert(2);
  tree.Insert(2);

  auto result_exist = tree.UpperBound(1);

  EXPECT_EQ(result_exist, first);

  auto result_not_exist = tree.UpperBound(2);

  EXPECT_EQ(result_not_exist, tree.end());
}

TEST(RBTreeTest, EqualRangeTest) {
  RBTree::RedBlackTree<int> tree;
  tree.Insert(1);
  auto first = tree.Insert(2);
  tree.Insert(2);
  auto last = tree.Insert(3);

  auto [from, to] = tree.EqualRange(2);

  EXPECT_EQ(from, first);
  EXPECT_EQ(to, last);

  {
    auto [first, last] = tree.EqualRange(1337);
    ASSERT_EQ(first, last);
  }
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

TEST_F(RBTreeTestF, DISABLED_ValidRBTreeWithDuplicatesAfterAllNodesDelete) {
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

TEST(RBTreeTest, RandomOperationsStressTest) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> key_dist(-1000, 1000);
  std::uniform_int_distribution<> op_dist(0, 1); // 0-remove, 1-insert

  RBTree::RedBlackTree<int> tree;
  std::set<int> reference;

  for (int i = 0; i < 10000; ++i) {
    int key = key_dist(gen);
    bool do_insert = op_dist(gen) != 0;

    bool was_empty = reference.empty();

    if (do_insert || was_empty) {
      tree.Insert(key);
      reference.insert(key);
    } else {
      tree.Remove(key);
      reference.erase(key);
    }

    auto msg = std::format("Failed on iteration {} when there was operation {}",
                           i, ((do_insert || was_empty) ? "insert" : "remove"));
    ASSERT_TRUE(test::RBTreeChecker::RootIsBlack(tree)) << msg;
    ASSERT_TRUE(test::RBTreeChecker::IsInorder(tree)) << msg;
    ASSERT_TRUE(test::RBTreeChecker::NoDoubleRed(tree)) << msg;
    ASSERT_TRUE(test::RBTreeChecker::SameBlackHeight(tree)) << msg;
    ASSERT_TRUE(test::RBTreeChecker::Validate(tree)) << msg;

    // Check that all keys exist
    for (int k : reference) {
      ASSERT_NE(tree.Find(k), tree.end());
    }
  }
}

TEST(RBTreeTest, FindFailingSeed) {
  const int kMaxSeed = 1000;
  for (int seed = 0; seed <= kMaxSeed; ++seed) {
    std::mt19937 gen(seed);
    RBTree::RedBlackTree<int> tree;

    for (int i = 0; i < kMaxSeed; ++i) {
      int key = gen() % 100 - 50; // [-50, 49]
      bool insert = ((gen() & 0x1) == 0);

      if (insert) {
        tree.Insert(key);
      } else {
        tree.Remove(key);
      }

      bool is_root_black = test::RBTreeChecker::RootIsBlack(tree);
      bool is_inorder = test::RBTreeChecker::IsInorder(tree);
      bool is_same_bh = test::RBTreeChecker::SameBlackHeight(tree);
      bool is_no_double_red = test::RBTreeChecker::NoDoubleRed(tree);

      if (!is_root_black || !is_inorder || !is_same_bh || !is_no_double_red) {
        std::cout << std::format(R"<1337>(
            
            FAILING SEED: {}
            Failed at iteration {}
            On operation {}
            
            )<1337>",
                                 seed, i, (insert ? "insert" : "remove"))
                  << std::endl;

        EXPECT_TRUE(is_root_black);
        EXPECT_TRUE(is_inorder);
        EXPECT_TRUE(is_same_bh);
        EXPECT_TRUE(is_no_double_red);
        return;
      }
    }

    if (seed % 100 == 0) {
      std::cout << "Tested seed " << seed << " - OK" << std::endl;
    }
  }
}