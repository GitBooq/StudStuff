#include <concepts>
#include <functional>
#include <iostream>
#include <iterator>
#include <ostream>
#include <utility>

namespace RBTree {
enum class Color { kRed, kBlack };

/**
 * @brief Red-Black Tree node class
 *
 * @tparam T type of data in node
 */
template <typename T> struct RBNode {
  T data;
  Color color;
  RBNode *parent;
  RBNode *left;
  RBNode *right;

  explicit RBNode(const T &value)
      : data(value), color(Color::kRed), parent(nullptr), left(nullptr),
        right(nullptr) {}
};

template <typename Key, typename Compare = std::less<Key>>
concept RedBlackTreeKey =
    std::movable<Key> &&
    std::strict_weak_order<Compare, const Key &, const Key &>;

/**
 * @brief Red-Black Tree container
 * @note Use new/delete to manage memory
 * @tparam Key
 * @tparam Compare
 */
template <typename Key, typename Compare = std::less<Key>>
  requires RedBlackTreeKey<Key, Compare>
class RedBlackTree {
  static_assert(std::is_default_constructible_v<Compare>,
                "Compare must be default constructible");

public:
  //============
  // Interface
  //============

  using value_type = Key;
  using size_type = std::size_t;
  using reference = const value_type &; // const
  using const_reference = const value_type &;
  using difference_type = std::ptrdiff_t;
  using NodeType = RBNode<Key>;

  RedBlackTree() noexcept(std::is_nothrow_default_constructible_v<Compare>) =
      default;
  ~RedBlackTree() noexcept { Clear(); }

  /// Deep copy ctor
  RedBlackTree(const RedBlackTree &other)
      : root_(CopyRecursive(other.root_, nullptr)), size_(other.size_),
        comp_(other.comp_) {}

  /// Deep copy assignment
  RedBlackTree &operator=(const RedBlackTree &other) {
    RedBlackTree tmp(other); // self-assign robust
    Swap(tmp);
    return *this;
  }

  RedBlackTree(RedBlackTree &&other) noexcept
      : root_(std::exchange(other.root_, nullptr)),
        size_(std::exchange(other.size_, 0)), comp_(std::move(other.comp_)) {}

  RedBlackTree &operator=(RedBlackTree &&other) noexcept {
    RedBlackTree tmp(std::move(other)); // self-move robust
    Swap(tmp);
    return *this;
  }

  // Iterators
  /// @todo add reverse iterators
  template <bool IsConst> class BasicIterator;
  using Iterator = BasicIterator<false>;
  using ConstIterator = BasicIterator<true>;

  Iterator begin() { return Iterator(GetMinimum(root_)); }
  Iterator end() { return Iterator(nullptr); }
  ConstIterator begin() const noexcept {
    return ConstIterator(GetMinimum(root_));
  }
  ConstIterator end() const noexcept { return ConstIterator(nullptr); }
  ConstIterator cbegin() const noexcept {
    return ConstIterator(GetMinimum(root_));
  }
  ConstIterator cend() const noexcept { return ConstIterator(nullptr); }

  /**
   * @brief Insert element with specified key into tree
   * @note Use expr new to allocate space on free store
   * @tparam K
   * @param val key to insert
   * @return Iterator to inserted element
   */
  template <typename K> Iterator Insert(K &&val);

  /**
   * @brief Find element with specified key, if several elements with this key
   * exist, return any of them
   *
   * @param key
   * @return Iterator
   */
  Iterator Find(const value_type &key) noexcept {
    return Iterator(FindNode(key));
  }
  ConstIterator Find(const value_type &key) const noexcept {
    return ConstIterator(FindNode(key));
  }

  /**
   * @brief Remove all elements with equal key
   *
   * @param key
   * @return size_type Number of elements removed
   */
  std::size_t Remove(value_type key) noexcept;

  /// Inorder print tree nodes to cout
  void Print(std::ostream &ostream = std::cout) const
    requires requires(std::ostream &ostream, value_type key) { ostream << key; }
  ;

  /// Clear tree
  void Clear() noexcept {
    ClearRecursive(root_);
    root_ = nullptr;
    size_ = 0;
  }

  /// Return true if tree is empty, false otherwise
  [[nodiscard]] bool empty() const noexcept { return root_ == nullptr; }

  /// Swap trees
  void Swap(RedBlackTree &other) noexcept {
    std::swap(root_, other.root_);
    std::swap(size_, other.size_);
    std::swap(comp_, other.comp_);
  }

  /// Returns the number of elements in the container
  [[nodiscard]] size_type size() const noexcept { return size_; }

private:
  NodeType *root_ = nullptr;
  size_type size_ = 0;
  Compare comp_;

  //============
  // Helpers
  //============

  /**
   * @brief Balance tree after insertion
   * @note Root is always black; No two consecutive red nodes; All paths from
   * root to leafs contain same amount of black nodes.
   * @param node
   */
  void InsertFixup(NodeType *node) noexcept;

  /**
   * @brief Balance tree after deleting black node
   * @note Root is always black; No two consecutive red nodes; All paths from
   * root to leafs contain same amount of black nodes.
   * @param node
   */
  void RemoveFixup(NodeType *node) noexcept;

  /// Return color of the node. Root(nullptr) is black
  [[nodiscard]] static Color ColorOf(NodeType *node) noexcept {
    return node ? node->color : Color::kBlack; // nullptr is black
  }

  /// Call appropriate rotate method based on bool flag
  void Rotate(NodeType *n, bool left);
  void RotateLeft(NodeType *x) noexcept;
  void RotateRight(NodeType *y) noexcept;

  /// Return child according to bool flag
  [[nodiscard]] NodeType *&GetChild(NodeType *n, bool left) {
    return left ? n->left : n->right;
  }

  /// Return node with specific key
  [[nodiscard]] NodeType *FindNode(const value_type &key) const noexcept;

  [[nodiscard]] NodeType *LowerBoundImpl(const value_type &key) const noexcept;

  /// Return iterator to first node with key >= specified key
  [[nodiscard]] Iterator LowerBound(const value_type &key) noexcept {
    auto result = LowerBoundImpl(key);
    return (result != nullptr) ? Iterator(result) : end();
  }

  /// Return const iterator to first node with key >= specified key
  [[nodiscard]] ConstIterator LowerBound(const value_type &key) const noexcept {
    auto result = LowerBoundImpl(key);
    return (result != nullptr) ? ConstIterator(result) : cend();
  }

  [[nodiscard]] NodeType *UpperBoundImpl(const value_type &key) const noexcept;

  /// Return iterator to first node with key > specified key
  [[nodiscard]] Iterator UpperBound(const value_type &key) noexcept {
    auto result = UpperBoundImpl(key);
    return (result != nullptr) ? Iterator(result) : end();
  }
  /// Return const iterator to first node with key > specified key
  [[nodiscard]] ConstIterator UpperBound(const value_type &key) const noexcept {
    auto result = UpperBoundImpl(key);
    return (result != nullptr) ? ConstIterator(result) : cend();
  }

  [[nodiscard]] std::pair<Iterator, Iterator>
  EqualRange(const value_type &key) noexcept {
    return {LowerBound(key), UpperBound(key)}; // тут почемуто вызво конс версий
  }

  /// Return iterators pair(range) of nodes with equal keys
  [[nodiscard]] std::pair<ConstIterator, ConstIterator>
  EqualRange(const value_type &key) const noexcept {
    return {LowerBound(key), UpperBound(key)};
  }

  /// Remove specified node
  void RemoveOne(NodeType *node) noexcept;

  /// Erase element by iterator. Return iterator to successor
  [[nodiscard]] Iterator
  Erase(ConstIterator pos) noexcept; // can be moved to public interface

  /// Get the leftmost node in subtree with root pointed to by %node
  [[nodiscard]] NodeType *GetMinimum(NodeType *node) const noexcept;

  /// Get the rightmost node in subtree with root pointed to by %node
  [[nodiscard]] NodeType *GetMaximum(NodeType *node) const noexcept;

  /// Transplant node u to node v
  void Transplant(NodeType *u, NodeType *v) noexcept;

  /// Recursively clear tree
  void ClearRecursive(NodeType *node) noexcept;

  /**
   * @brief Return root of deep copy of subtree with root specified by %node
   *
   * @param node subtree root
   * @param parent new parent for copy
   * @return NodeType*
   */
  NodeType *CopyRecursive(const NodeType *node, NodeType *parent);
};

// ===============
// Implementation
// ===============

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
template <typename K>
typename RedBlackTree<Key, Compare>::Iterator
RedBlackTree<Key, Compare>::Insert(K &&val) {
  if (empty()) {
    root_ = new NodeType(std::forward<K>(val)); // red node
    ++size_;
    root_->color = Color::kBlack; // root is always black
    return Iterator(root_);
  }

  NodeType *current = root_;
  NodeType *parent = nullptr;

  // Find place to insert
  while (current) {
    parent = current;
    auto cur_data = current->data;
    if (comp_(val, cur_data)) {
      current = current->left;
    } else {
      current = current->right; // duplicates go to right too
    }
  }

  // Insert
  NodeType *new_node = new NodeType(std::forward<K>(val));
  new_node->parent = parent;
  ++size_;
  if (comp_(new_node->data, parent->data)) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
  }

  // Balancing
  InsertFixup(new_node);

  return Iterator(new_node);
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
std::size_t RedBlackTree<Key, Compare>::Remove(Key key) noexcept {
  auto removed_cnt = 0UZ;

  auto [first, last] = EqualRange(key);
  while (first != last) {
    ++removed_cnt;
    first = Erase(first);
  }
  return removed_cnt;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::Print(std::ostream &ostream) const
  requires requires(std::ostream &ostream, Key key) { ostream << key; }
{
  std::copy(cbegin(), cend(), std::ostream_iterator<Key>(ostream, " "));
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::InsertFixup(NodeType *node) noexcept {
  // If the parent of the new node is black, no properties are violated.
  // While parent red - unbalanced
  while (node != root_ && node->parent->color == Color::kRed) {
    NodeType *parent = node->parent;
    NodeType *grandparent = parent->parent;

    // Case A: parent is grandparent's left kid
    if (parent == grandparent->left) {
      NodeType *uncle = grandparent->right;

      // Case 1: Red uncle
      // Recolor parent and uncle to black, grandparent to red. Then, move up
      // the tree.
      if (uncle && uncle->color == Color::kRed) {
        parent->color = Color::kBlack;
        uncle->color = Color::kBlack;
        grandparent->color = Color::kRed;
        node = grandparent;
      }
      // Case 2: Black uncle
      // If node is a right child, perform a left rotation on the parent. If the
      // node is a left child, perform a right rotation on the grandparent and
      // recolor.
      else {
        // Case 2a: node is right child -> rotate left
        if (node == parent->right) {
          node = parent;
          RotateLeft(node);
          parent = node->parent;
          grandparent = parent->parent;
        }
        // Case 2b: node is left child -> rotate right
        parent->color = Color::kBlack;
        grandparent->color = Color::kRed;
        RotateRight(grandparent);
      }
    }
    // Case B: parent is grandparent's right kid
    else {
      NodeType *uncle = grandparent->left;

      if (uncle && uncle->color == Color::kRed) {
        parent->color = Color::kBlack;
        uncle->color = Color::kBlack;
        grandparent->color = Color::kRed;
        node = grandparent;
      } else {
        if (node == parent->left) {
          node = parent;
          RotateRight(node);
          parent = node->parent;
          grandparent = parent->parent;
        }
        parent->color = Color::kBlack;
        grandparent->color = Color::kRed;
        RotateLeft(grandparent);
      }
    }
  }

  // Root is always black
  root_->color = Color::kBlack;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
typename RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::FindNode(const Key &key) const noexcept {
  NodeType *current = root_;
  while (current) {
    Key cur_key = current->data;
    if (comp_(key, cur_key)) {
      current = current->left;
    } else if (comp_(cur_key, key)) {
      current = current->right;
    } else {
      return current;
    }
  }
  return nullptr;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::LowerBoundImpl(
    const value_type &key) const noexcept {
  auto current = root_;
  NodeType *result = nullptr;

  while (current) {
    if (comp_(current->data, key)) { // current < target
      current = current->right;
    } else { // current >= target
      result = current;
      current = current->left;
    }
  }

  return result;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::UpperBoundImpl(
    const value_type &key) const noexcept {
  auto current = root_;
  NodeType *result = nullptr;

  while (current) {
    if (comp_(key, current->data)) { // target < current
      result = current;
      current = current->left;
    } else { // target >= current
      current = current->right;
    }
  }

  return result;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::RemoveOne(NodeType *node) noexcept {
  if (node == nullptr) {
    return;
  }

  NodeType *y = node;    // the node that will be removed
  NodeType *x = nullptr; // the child that replaces y (may be nullptr)
  Color original_color = y->color;

  // Case 1: No left child
  if (node->left == nullptr) {
    x = node->right;
    Transplant(node, node->right);
  }
  // Case 2: No right child
  else if (node->right == nullptr) {
    x = node->left;
    Transplant(node, node->left);
  }
  // Case 3: Two children
  else {
    // Find successor
    y = GetMinimum(node->right);
    original_color = y->color;
    x = y->right; // x will take y's place after removal

    // If y is direct child of node
    if (y->parent == node) {
      if (x)
        x->parent = y;         // x becomes child of y
    } else {                   // y is somewhere deeper in right subtree
      Transplant(y, y->right); //  Extract y from its position
      y->right = node->right;  // Move node's right child to y
      y->right->parent = y;
    }

    // Replace node with y
    Transplant(node, y);
    // Move node's left child to y
    y->left = node->left;
    y->left->parent = y;
    // Save original color of node
    y->color = node->color;
  }

  delete node;
  --size_;

  // Fix tree if the deleted node was black
  if (original_color == Color::kBlack) {
    RemoveFixup(x);
  }
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::RemoveFixup(NodeType *node) noexcept {
  if (node == nullptr) {
    return;
  }
  while (node != root_ && ColorOf(node) == Color::kBlack) {
    NodeType *parent = node->parent;
    bool left = (node == parent->left); // node direction related to parent
    bool right = !left;                 // opposite direction

    NodeType *&sibling = GetChild(parent, right);
    if (sibling == nullptr) {
      node = parent;
      continue;
    }
    // Case 1: Red sibling
    if (ColorOf(sibling) == Color::kRed) {
      sibling->color = Color::kBlack;
      parent->color = Color::kRed;
      Rotate(parent, left);              // rotate to node
      sibling = GetChild(parent, right); // update sibling
    }
    // Case 2: Black siblings
    if (ColorOf(sibling->left) == Color::kBlack &&
        ColorOf(sibling->right) == Color::kBlack) {
      sibling->color = Color::kRed;
      node = parent; // move up
    } else {
      bool outer_black = (left ? ColorOf(sibling->right) == Color::kBlack
                               : ColorOf(sibling->left) == Color::kBlack);
      // Case 3: Black outer nephew
      if (outer_black) {
        // Recolor inner nephew and sibling, rotate sibling
        NodeType *&inner_nephew = GetChild(sibling, left);
        if (inner_nephew) {
          inner_nephew->color = Color::kBlack;
        }
        sibling->color = Color::kRed;
        Rotate(sibling, right);            // opposite rotatation
        sibling = GetChild(parent, right); // update sibling
      }
      // Case 4: Red outer nephew
      sibling->color = parent->color;
      parent->color = Color::kBlack;
      NodeType *&outer_nephew = GetChild(sibling, right);
      if (outer_nephew) {
        outer_nephew->color = Color::kBlack;
      }
      Rotate(parent, left);
      node = root_; // end cycle
    }
  }
  // root is always black
  if (node != nullptr) {
    node->color = Color::kBlack;
  }
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::RotateLeft(NodeType *x) noexcept {
  // x - the node that falls down to the left
  NodeType *y = x->right; // y - right child of x, moves up to x's position

  // Step 1: Move y's left child to x's right
  x->right = y->left;
  if (y->left) {
    y->left->parent = x;
  }

  // Step 2: Move y up to x's position
  y->parent = x->parent;
  if (!x->parent) {
    root_ = y; // x was root, now y becomes root
  } else if (x == x->parent->left) {
    x->parent->left = y; // x was left child
  } else {
    x->parent->right = y; // x was right child
  }

  // Step 3: Make x the left child of y
  y->left = x;
  x->parent = y;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::RotateRight(NodeType *y) noexcept {
  // y - the node that falls down to the right
  NodeType *x = y->left; // x - left child of y, moves up to y's position

  // Step 1: Move x's right child to y's left
  y->left = x->right;
  if (x->right) {
    x->right->parent = y;
  }

  // Step 2: Move x up to y's position
  x->parent = y->parent;
  if (!y->parent) {
    root_ = x; // y was root, now x becomes root
  } else if (y == y->parent->left) {
    y->parent->left = x; // y was left child
  } else {
    y->parent->right = x; // y was right child
  }

  // Step 3: Make y the right child of x
  x->right = y;
  y->parent = x;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::Rotate(NodeType *n, bool left) {
  if (left) {
    RotateLeft(n);
  } else {
    RotateRight(n);
  }
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
typename RedBlackTree<Key, Compare>::Iterator
RedBlackTree<Key, Compare>::Erase(ConstIterator pos) noexcept {
  if (pos == end()) {
    return Iterator(nullptr);
  }

  NodeType *node = const_cast<NodeType *>(pos.node_);
  auto next = Iterator(Iterator::GetSuccessor(node));

  RemoveOne(node);

  return next;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
typename RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::GetMinimum(NodeType *node) const noexcept {
  while (node && node->left) {
    node = node->left;
  }
  return node;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
typename RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::GetMaximum(NodeType *node) const noexcept {
  while (node && node->right) {
    node = node->right;
  }
  return node;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::Transplant(NodeType *u, NodeType *v) noexcept {
  // u's parent becomes v's parent
  if (u->parent == nullptr) {
    root_ = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }

  // update parent for v
  if (v != nullptr) {
    v->parent = u->parent;
  }
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
void RedBlackTree<Key, Compare>::ClearRecursive(NodeType *node) noexcept {
  if (node == nullptr) {
    return;
  }
  ClearRecursive(node->left);
  ClearRecursive(node->right);
  delete node;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
typename RedBlackTree<Key, Compare>::NodeType *
RedBlackTree<Key, Compare>::CopyRecursive(const NodeType *node,
                                          NodeType *parent) {
  if (node == nullptr) {
    return nullptr;
  }

  NodeType *new_node = nullptr;
  try {
    new_node = new NodeType(node->data);
    new_node->color = node->color;
    new_node->parent = parent;
    new_node->left = CopyRecursive(node->left, new_node);
    new_node->right = CopyRecursive(node->right, new_node);
  } catch (...) {
    ClearRecursive(new_node);
    throw;
  }
  return new_node;
}

template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
template <bool IsConst>
class RedBlackTree<Key, Compare>::BasicIterator final {
public:
  using NodePtr = std::conditional_t<IsConst, const NodeType *, NodeType *>;

  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const Key &; // always const
  using pointer = const Key *;   // always const

  BasicIterator() noexcept : node_(nullptr) {}
  explicit BasicIterator(NodePtr node) noexcept : node_(node) {}

  // NOLINTBEGIN(google-explicit-constructor)
  /**
   * @brief Converting ctor
   * @note Construct const iterator from non const
   * @tparam OtherIsConst
   */
  template <bool OtherIsConst>
    requires(IsConst && !OtherIsConst)
  BasicIterator(const BasicIterator<OtherIsConst> &other) noexcept
      : node_(other.node_) {}
  // NOLINTEND(google-explicit-constructor)

  BasicIterator(const BasicIterator &) = default;
  BasicIterator &operator=(const BasicIterator &) = default;

  reference operator*() const noexcept { return node_->data; }
  pointer operator->() const noexcept { return &node_->data; }

  BasicIterator &operator++() noexcept {
    node_ = GetSuccessor(node_);
    return *this;
  }

  BasicIterator operator++(int) noexcept {
    BasicIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  BasicIterator &operator--() noexcept {
    node_ = GetPredecessor(node_);
    return *this;
  }

  BasicIterator operator--(int) noexcept {
    BasicIterator tmp = *this;
    --(*this);
    return tmp;
  }

  auto operator<=>(const BasicIterator &) const noexcept = default;

  // Give access to all specialisations
  template <bool> friend class BasicIterator;

private:
  friend class RedBlackTree;

  NodePtr node_; ///< node ptr iterator is pointing to

  // Helpers

  /**
   * @brief Get next node
   *
   * @param node
   * @return NodePtr
   */
  static NodePtr GetSuccessor(NodePtr node) noexcept {
    if (node == nullptr) {
      return nullptr;
    }

    // If right child exist: go one right then left till the end
    if (node->right) {
      node = node->right;
      if (node->right != nullptr && node->right->data == node->data) {
        return node->right; // return duplicate
      }
      while (node->left) {
        node = node->left;
      }
      return node;
    }

    // If no right child exist: go up
    NodePtr parent = node->parent;
    while (parent && node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }

  /**
   * @brief Get prev node
   *
   * @param node
   * @return NodePtr
   */
  static NodePtr GetPredecessor(NodePtr node) noexcept {
    if (node == nullptr) {
      return nullptr;
    }

    // If left child exist: go one left then right till the end
    if (node->left) {
      node = node->left;
      while (node->right) {
        node = node->right;
      }
      return node;
    }

    // If no left child exist: go up
    NodePtr parent = node->parent;
    while (parent && node == parent->left) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }
};

/// STL ADL swap
template <typename Key, typename Compare>
  requires RedBlackTreeKey<Key, Compare>
inline void swap(RedBlackTree<Key, Compare> &first,
                 RedBlackTree<Key, Compare> &second) {
  first.Swap(second);
}
} // namespace RBTree