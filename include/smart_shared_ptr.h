/**
 * @file smart_shared_ptr.h
 * @brief SharedPtr and WeakPtr implementation with reference counting.
 *
 * This file contains the implementation of shared_ptr and weak_ptr
 * smart pointers with full support for:
 * - Reference counting (shared ownership)
 * - Weak pointers (non-owning observers)
 * - Custom deleters
 * - One-dimension Arrays support
 * - Polymorphic conversions
 * - Exception safety (strong guarantee)
 *
 * @see SharedPtr
 * @see WeakPtr
 * @see control_block.h
 */

#pragma once

#include <compare>     // for compare_three_way
#include <concepts>    // for move_constructible
#include <cstddef>     // for nullptr_t, size_t, ptrdiff_t
#include <memory>      // for default_delete
#include <type_traits> // for remove_extent_t
#include <utility>     // for exchange, swap

#include "control_block.h" // for CbBase, CbRegular

namespace my::memory {
// ============================================================================
// Concepts and type aliases
// ============================================================================

/**
 * @brief Concept for convertible and complete types.
 *
 * Ensures that From* can be implicitly converted to To*
 * and that From is a complete type (sizeof can only be applied to
 * complete types).
 *
 * @tparam From Source type.
 * @tparam To Target type.
 */
template <typename From, typename To>
concept ConvertibleAndComplete =
    std::is_convertible_v<From *, To *> && requires { sizeof(From); };

/**
 * @brief Check convertaion between smart pointers element types
 *
 * @tparam From
 * @tparam To
 */
template <typename From, typename To>
concept CompatibleSmartPtr = requires {
  typename From::element_type;
  typename To::element_type;
  requires std::is_convertible_v<typename From::element_type *,
                                 typename To::element_type *>;
};

/**
 * @brief T is not array
 */
template <typename T>
concept NotArray = (!std::is_array_v<T>);

/**
 * @brief Type alias constrained for unbounded array types.
 *
 * This alias is valid only when T is unbounded array (T[]).
 *
 * @tparam T Type to check.
 */
template <typename T>
  requires std::is_array_v<T> && (std::extent_v<T> == 0)
using UnboundedArray = T;

/**
 * @brief T is array
 */
template <typename T>
concept Array = std::is_array_v<T>;

// ============================================================================
// SharedPtrBase - Common functionality for all shared pointers
// ============================================================================

/**
 * @brief Base class for shared pointer implementation.
 *
 * Provides reference counting functionality common to both
 * SharedPtr<T> and SharedPtr<T[]>.
 *
 * @details
 * Manages the control block pointer and provides:
 * - Copy/move operations with correct reference counting
 * - Reference count queries
 * - Boolean conversion
 * - Swap functionality
 */
class SharedPtrBase {
protected:
  CbBase *cb_{nullptr}; ///< Pointer to the control block

  /**
   * @brief Increments the strong reference count.
   */
  void AddRef() noexcept {
    if (cb_ != nullptr) {
      cb_->AddRef();
    }
  }

  /**
   * @brief Decrements the strong reference count.
   *
   * May destroy the managed object and control block
   * if the count reaches zero.
   */
  void Release() noexcept {
    if (cb_ != nullptr) {
      cb_->Release();
    }
  }

  /**
   * @brief Releases the control block.
   *
   * Calls Release() and sets cb_ to nullptr.
   */
  void Reset() noexcept {
    Release();
    cb_ = nullptr;
  }

public:
  /**
   * @brief Constructs an empty SharedPtrBase.
   */
  constexpr SharedPtrBase() noexcept : cb_(nullptr) {}

  /**
   * @brief Constructs a SharedPtrBase from a control block.
   *
   * @param b Pointer to the control block (may be nullptr).
   */
  explicit constexpr SharedPtrBase(CbBase *cblock) noexcept : cb_(cblock) {}

  /**
   * @brief Copy constructor.
   *
   * @param other The SharedPtrBase to copy.
   *
   * @details Increments the reference count of the managed object.
   */
  SharedPtrBase(const SharedPtrBase &other) noexcept : cb_(other.cb_) {
    AddRef();
  }

  /**
   * @brief Move constructor.
   *
   * @param other The SharedPtrBase to move from.
   *
   * @details The other object becomes empty.
   */
  SharedPtrBase(SharedPtrBase &&other) noexcept
      : cb_{std::exchange(other.cb_, nullptr)} {}

  /**
   * @brief Destructor.
   *
   * Decrements the reference count. If this was the last
   * shared pointer, the managed object is destroyed.
   */
  ~SharedPtrBase() noexcept { Release(); }

  /**
   * @brief Copy/move assignment operator.
   *
   * @param other The SharedPtrBase to assign from.
   * @return SharedPtrBase& Reference to this object.
   *
   * @details Uses copy-and-swap idiom for strong exception safety.
   */
  SharedPtrBase &operator=(SharedPtrBase other) noexcept {
    Swap(other);
    return *this;
  }

  /**
   * @brief Swaps the control blocks with another SharedPtrBase.
   *
   * @param other The SharedPtrBase to swap with.
   */
  void Swap(SharedPtrBase &other) noexcept { std::swap(cb_, other.cb_); }

  /**
   * @brief Returns the strong reference count.
   *
   * @return size_t Number of shared_ptrs owning the object,
   *         or 0 if empty.
   */
  size_t use_count() const noexcept {
    return (cb_ != nullptr) ? cb_->use_count() : 0;
  }
};

// SharedPtr - Primary dummy template
template <typename T> class SharedPtr {
  static_assert(
      sizeof(T) == 0,
      "SharedPtr supports only non-array types and one-dimensional arrays");
};

// ============================================================================
// SharedPtr - Specialization for non-array types
// ============================================================================

/**
 * @brief Shared pointer with reference counting.
 *
 * Implements shared ownership semantics. Multiple SharedPtr instances
 * can own the same object. The object is destroyed when the last
 * owning SharedPtr is destroyed or reset.
 *
 * @tparam T Type of the managed object.
 *
 * @details
 * Features:
 * - Copyable and movable
 * - Supports custom deleters
 * - Supports polymorphic conversions
 * - Works with weak pointers
 * - Strong exception safety
 *
 * @see WeakPtr
 */
template <NotArray T> class SharedPtr<T> : public SharedPtrBase {
private:
  /**
   * @brief All WeakPtr specializations have access to private members
   *
   * Required for WeakPtr::lock() to access private constructor
   */
  template <typename> friend class WeakPtr;

public:
  using element_type = T;

private:
  element_type *ptr_{nullptr}; ///< Stored pointer to the managed object

  /**
   * @brief Private constructor for WeakPtr::lock().
   *
   * @param p Pointer to the managed object.
   * @param block Control block to share.
   */
  SharedPtr(T *ptr, CbBase *block) noexcept : SharedPtrBase{block}, ptr_{ptr} {
    if (cb_ != nullptr) {
      cb_->AddRef();
    }
  }

public:
  // ========================================================================
  // Constructors and destructor
  // ========================================================================

  /**
   * @brief Constructs an empty SharedPtr.
   */
  constexpr SharedPtr() : SharedPtrBase(), ptr_(nullptr) {}

  /**
   * @brief Constructs an empty SharedPtr from nullptr.
   * @note Allow implicit conversion
   */
  constexpr SharedPtr(std::nullptr_t) noexcept : SharedPtr() {}

  /**
   * @brief Constructs a SharedPtr from a raw pointer.
   *
   * @param p Raw pointer to manage.
   *
   * @throws std::bad_alloc If control block allocation fails.
   *
   * @warning The pointer must be allocated with `new` (or compatible).
   */
  explicit SharedPtr(T *ptr)
      : SharedPtrBase(ptr ? new CbRegular<T>(ptr) : nullptr), ptr_(ptr) {}

  /**
   * @brief Constructs a SharedPtr from nullptr and custom deleter.
   *
   * @param d Deleter.
   *
   * @throws std::bad_alloc If control block allocation fails.
   */
  template <std::move_constructible Deleter>
  SharedPtr(std::nullptr_t, Deleter &&del)
      : SharedPtrBase(new CbRegular<T, std::decay_t<Deleter>>(
            nullptr, std::forward<Deleter>(del))),
        ptr_(nullptr) {}

  /**
   * @brief Constructs a SharedPtr from a raw pointer and custom deleter.
   *
   * @param p Raw pointer to manage.
   * @param d Deleter.
   *
   * @throws std::bad_alloc If control block allocation fails.
   *
   * @warning The pointer must be allocated with `new` (or compatible).
   */
  template <std::move_constructible Deleter>
  SharedPtr(T *ptr, Deleter &&del)
      : SharedPtrBase(ptr ? new CbRegular<T, std::decay_t<Deleter>>(
                                ptr, std::forward<Deleter>(del))
                          : nullptr),
        ptr_(ptr) {}

  /**
   * @brief Copy constructor.
   *
   * @param other The SharedPtr to copy.
   *
   * @details Increments the reference count.
   */
  SharedPtr(const SharedPtr &other) noexcept
      : SharedPtrBase(other), ptr_(other.ptr_) {}

  /**
   * @brief Converting copy constructor.
   *
   * @tparam U Type of the other SharedPtr.
   * @param other The SharedPtr to copy.
   *
   * @note Allow implicit conversion
   *
   * @details Enabled only if U* is convertible to T*.
   *          Supports polymorphic conversions (Derived* to Base*).
   */
  template <NotArray U>
  SharedPtr(const SharedPtr<U> &other) noexcept
      : SharedPtrBase(other), ptr_(other.Get()) {
    static_assert(std::is_convertible_v<U *, T *>,
                  "U* must be convertible to T*");
  }

  /**
   * @brief Move constructor.
   *
   * @param other The SharedPtr to move from.
   *
   * @details The other object becomes empty.
   */
  SharedPtr(SharedPtr &&other) noexcept
      : SharedPtrBase(std::move(other)),
        ptr_{std::exchange(other.ptr_, nullptr)} {}

  /**
   * @brief Destructor.
   */
  ~SharedPtr() noexcept = default;

  // ========================================================================
  // Assignment
  // ========================================================================

  /**
   * @brief Copy/move assignment operator.
   *
   * @param other The SharedPtr to assign from.
   * @return SharedPtr& Reference to this object.
   *
   * @details Uses copy-and-swap idiom for strong exception safety.
   *          Works for both copy and move assignment.
   */
  SharedPtr &operator=(SharedPtr other) noexcept {
    Swap(other);
    return *this;
  }

  // ========================================================================
  // Modifiers
  // ========================================================================

  /**
   * @brief Releases ownership of the managed object.
   *
   * Set ptr_ of managed object to nullptr.
   * If this was the last SharedPtr owning the object,
   * the object is destroyed.
   */
  void Reset() noexcept {
    SharedPtrBase::Reset();
    ptr_ = nullptr;
  }

  /**
   * @brief Replaces the managed object with a new pointer.
   *
   * @tparam Y Type of the new pointer.
   * @param p New raw pointer to manage.
   *
   * @throws std::bad_alloc If control block allocation fails.
   *
   * @details The previous object is released.
   */
  template <ConvertibleAndComplete<T> Y> void Reset(Y *ptr) {
    SharedPtr<T> tmp{ptr};
    Swap(tmp);
  }

  // ========================================================================
  // Observers
  // ========================================================================

  /**
   * @brief Dereferences the stored pointer.
   *
   * @return T& Reference to the managed object.
   * @warning Undefined behavior if the pointer is nullptr.
   */
  element_type &operator*() const noexcept { return *Get(); }

  /**
   * @brief Accesses members of the managed object.
   *
   * @return T* Pointer to the managed object.
   * @warning Undefined behavior if the pointer is nullptr.
   */
  element_type *operator->() const noexcept { return Get(); }

  /**
   * @brief Checks if the shared pointer owns an object.
   *
   * @return true if the shared pointer stores non-null pointer, false otherwise
   */
  explicit operator bool() const noexcept { return Get() != nullptr; }

  /**
   * @brief Returns the stored pointer.
   *
   * @return T* The stored pointer (may be nullptr).
   */
  element_type *Get() const noexcept { return ptr_; }

  // ========================================================================
  // Modifiers
  // ========================================================================

  /**
   * @brief Swaps the contents with another SharedPtr.
   *
   * @param other The SharedPtr to swap with.
   */
  void Swap(SharedPtr &other) noexcept {
    SharedPtrBase::Swap(other);
    std::swap(ptr_, other.ptr_);
  }
};

// ============================================================================
// SharedPtr - Array specialization (T[] or T[N])
// ============================================================================

/**
 * @brief Shared pointer specialization for arrays.
 *
 * @tparam T Type of array elements.
 *
 * @details Provides array-specific interface:
 * - operator[] for element access
 * - Uses delete[] by default
 *
 */
template <Array T> class SharedPtr<T> : public SharedPtrBase {
public:
  using element_type = std::remove_extent_t<T>;

private:
  /**
   * @brief All WeakPtr specializations have access to private members
   *
   * Required for WeakPtr::lock() to access private constructor
   */
  template <typename> friend class WeakPtr;

  using default_deleter = std::default_delete<element_type[]>;

  element_type *ptr_{nullptr}; ///< Pointer to the first element of the array

  /**
   * @brief Private constructor for WeakPtr::lock().
   *
   * @param p Pointer to the first element.
   * @param block Control block to share.
   */
  SharedPtr(element_type *ptr, CbBase *block) noexcept
      : SharedPtrBase{block}, ptr_{ptr} {
    if (cb_ != nullptr) {
      cb_->AddRef();
    }
  }

public:
  // ========================================================================
  // Constructors and destructor
  // ========================================================================

  /**
   * @brief Constructs an empty SharedPtr.
   */
  constexpr SharedPtr() noexcept : SharedPtrBase(), ptr_(nullptr) {}

  /**
   * @brief Constructs an empty SharedPtr from nullptr.
   *
   * @note Allow implicit conversion.
   */
  constexpr SharedPtr(std::nullptr_t) noexcept : SharedPtr() {}

  /**
   * @brief Constructs a SharedPtr from a raw array pointer.
   *
   * @param p Raw pointer to the array.
   *
   * @throws std::bad_alloc If control block allocation fails.
   *
   * @warning The pointer must be allocated with `new[]`.
   */
  explicit SharedPtr(element_type *ptr)
      : SharedPtrBase(ptr ? new CbRegular<element_type, default_deleter>(
                                ptr, default_deleter{})
                          : nullptr),
        ptr_(ptr) {}

  /**
   * @brief Constructs a SharedPtr from a nullptr and a custom deleter.
   *
   * @param d Deleter.
   *
   * @throws std::bad_alloc If control block allocation fails.
   */
  template <std::move_constructible Deleter>
  SharedPtr(std::nullptr_t, Deleter &&del)
      : SharedPtrBase(new CbRegular<element_type, std::decay_t<Deleter>>(
            nullptr, std::forward<Deleter>(del))),
        ptr_(nullptr) {}

  /**
   * @brief Constructs a SharedPtr from an array pointer and a custom deleter.
   *
   * @param p Pointer to the first element of the array to manage.
   * @param d Deleter.
   *
   * @throws std::bad_alloc If control block allocation fails.
   *
   * @warning The pointer must be allocated with `new` (or compatible).
   */
  template <std::move_constructible Deleter>
  SharedPtr(element_type *ptr, Deleter &&del)
      : SharedPtrBase(ptr ? new CbRegular<element_type, std::decay_t<Deleter>>(
                                ptr, std::forward<Deleter>(del))
                          : nullptr),
        ptr_(ptr) {}

  /**
   * @brief Copy constructor.
   *
   * @param other The SharedPtr to copy.
   */
  SharedPtr(const SharedPtr &other) noexcept
      : SharedPtrBase(other), ptr_(other.ptr_) {}

  /**
   * @brief Converting copy constructor.
   *
   * @tparam U Type of the other SharedPtr.
   * @param other The SharedPtr to copy.
   *
   * @note Allow implicit conversion.
   */
  template <Array U>
  SharedPtr(const SharedPtr<U> &other) noexcept
      : SharedPtrBase(other), ptr_(other.Get()) {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types are not convertible");
  }

  /**
   * @brief Move constructor.
   *
   * @param other The SharedPtr to move from.
   */
  SharedPtr(SharedPtr &&other) noexcept
      : SharedPtrBase(std::move(other)),
        ptr_{std::exchange(other.ptr_, nullptr)} {}

  /**
   * @brief Destructor.
   */
  ~SharedPtr() noexcept = default;

  // ========================================================================
  // Assignment
  // ========================================================================

  /**
   * @brief Copy/move assignment operator.
   *
   * @param other The SharedPtr to assign from.
   * @return SharedPtr& Reference to this object.
   */
  SharedPtr &operator=(SharedPtr other) noexcept {
    Swap(other);
    return *this;
  }

  // ========================================================================
  // Modifiers
  // ========================================================================

  /**
   * @brief Releases ownership of the managed array.
   */
  void Reset() noexcept {
    SharedPtrBase::Reset();
    ptr_ = nullptr;
  }

  /**
   * @brief Replaces the managed array with a new pointer.
   *
   * @tparam Y Type of the new pointer.
   * @param p New raw pointer to manage.
   */
  template <typename Y>
    requires std::is_convertible_v<Y *, element_type *>
  void Reset(Y *ptr) {
    SharedPtr<T> tmp{ptr}; // can throw
    Swap(tmp);
  }

  // ========================================================================
  // Observers
  // ========================================================================

  /**
   * @brief Checks if the shared pointer owns an object.
   *
   * @return true if the shared pointer stores non-null pointer, false otherwise
   */
  explicit operator bool() const noexcept { return Get() != nullptr; }

  /**
   * @brief Returns the stored pointer.
   *
   * @return T* Pointer to the first element (may be nullptr).
   */
  element_type *Get() const noexcept { return ptr_; }

  /**
   * @brief Accesses an element of the array.
   *
   * @param idx Index of the element.
   * @return T& Reference to the element.
   * @warning Undefined behavior if idx is out of bounds.
   */
  element_type &operator[](std::ptrdiff_t idx) const { return Get()[idx]; }

  // ========================================================================
  // Modifiers
  // ========================================================================

  /**
   * @brief Swaps the contents with another SharedPtr.
   *
   * @param other The SharedPtr to swap with.
   */
  void Swap(SharedPtr &other) noexcept {
    SharedPtrBase::Swap(other);
    std::swap(ptr_, other.ptr_);
  }
};

/**
 * @brief
 *
 */
template <typename T>
inline void swap(SharedPtr<T> &first, SharedPtr<T> &second) noexcept {
  first.Swap(second);
}

// ============================================================================
// WeakPtr
// ============================================================================

/**
 * @brief Weak pointer that observes an object managed by SharedPtr.
 *
 * Does not affect the object's lifetime. Can be used to break
 * circular references between shared_ptrs.
 *
 * @tparam T Type of the observed object.
 *
 * @details
 * Features:
 * - Non-owning observer
 * - Can be converted to SharedPtr if the object still exists (lock())
 * - Expires when the object is destroyed
 *
 * @see SharedPtr
 */
template <typename T> class WeakPtr {
private:
  template <typename> friend class WeakPtr;

public:
  // Underlying type for arrays
  using element_type = std::remove_extent_t<T>;

private:
  CbBase *cb_{nullptr}; ///< Pointer to the control block

  /**
   * @brief Increments the weak reference count.
   */
  void AddWeak() noexcept {
    if (cb_ != nullptr) {
      cb_->AddWeak();
    }
  }

  /**
   * @brief Decrements the weak reference count.
   */
  void ReleaseWeak() noexcept {
    if (cb_ != nullptr) {
      cb_->ReleaseWeak();
    }
  }

public:
  // ========================================================================
  // Constructors and destructor
  // ========================================================================

  /**
   * @brief Constructs an empty WeakPtr.
   */
  constexpr WeakPtr() noexcept : cb_{nullptr} {}

  /**
   * @brief Constructs an empty WeakPtr from nullptr.
   *
   * @note Allow implicit conversion.
   */
  constexpr WeakPtr(std::nullptr_t) noexcept : WeakPtr() {}

  /**
   * @brief Copy constructor.
   *
   * @param other The WeakPtr to copy.
   */
  WeakPtr(const WeakPtr &other) noexcept : cb_{other.cb_} { AddWeak(); }

  /**
   * @brief Converting copy constructor.
   *
   * @tparam U Type of the other WeakPtr.
   * @param other The WeakPtr to copy.
   *
   * @note Allow implicit conversion.
   */
  template <typename U>
  WeakPtr(const WeakPtr<U> &other) noexcept : cb_{other.cb_} {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types must be convertible");
    AddWeak();
  }

  /**
   * @brief Constructs a WeakPtr from a SharedPtr.
   *
   * @tparam U Type of the shared pointer.
   * @param shared The SharedPtr to observe.
   *
   * @note Allow implicit conversion.
   */
  template <typename U>
    requires(std::is_array_v<T> == std::is_array_v<U>)
  WeakPtr(const SharedPtr<U> &shared) noexcept : cb_{shared.cb_} {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types must be convertible");
    AddWeak();
  }

  /**
   * @brief Move constructor.
   *
   * @param other The WeakPtr to move from.
   */
  WeakPtr(WeakPtr &&other) noexcept : cb_{std::exchange(other.cb_, nullptr)} {}

  /**
   * @brief Converting move constructor.
   *
   * @tparam U Type of the other WeakPtr.
   * @param other The WeakPtr to move from.
   *
   * @note Allow implicit conversion.
   */
  template <typename U>
    requires(std::is_array_v<T> == std::is_array_v<U>)
  WeakPtr(WeakPtr<U> &&other) noexcept
      : cb_{std::exchange(other.cb_, nullptr)} {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types must be convertible");
  }

  /**
   * @brief Destructor.
   *
   * Decrements the weak reference count. Does not affect the managed object.
   */
  ~WeakPtr() noexcept { ReleaseWeak(); }

  // ========================================================================
  // Assignment
  // ========================================================================

  /**
   * @brief Copy assignment operator.
   *
   * @param other The WeakPtr to assign from.
   * @return WeakPtr& Reference to this object.
   */
  WeakPtr &operator=(WeakPtr other) noexcept {
    Swap(other);
    return *this;
  }

  /**
   * @brief Converting assignment operator.
   *
   * @tparam U Type of the other WeakPtr.
   * @param other The WeakPtr to assign from.
   * @return WeakPtr& Reference to this object.
   */
  template <class U>
    requires(std::is_array_v<T> == std::is_array_v<U>)
  WeakPtr &operator=(WeakPtr<U> other) noexcept {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types must be convertible");
    Swap(other);
    return *this;
  }

  /**
   * @brief Assigns from a SharedPtr.
   *
   * @tparam U Type of the shared pointer.
   * @param shared The SharedPtr to observe.
   * @return WeakPtr& Reference to this object.
   */
  template <class U>
    requires(std::is_array_v<T> == std::is_array_v<U>)
  WeakPtr &operator=(const SharedPtr<U> &shared) noexcept {
    using U_element = std::remove_extent_t<U>;
    static_assert(std::is_convertible_v<U_element *, element_type *>,
                  "Element types must be convertible");
    WeakPtr temp(shared);
    Swap(temp);
    return *this;
  }

  // ========================================================================
  // Modifiers
  // ========================================================================

  /**
   * @brief Releases the reference to the managed object.
   *
   * After this call, expired() returns true.
   */
  void Reset() noexcept {
    ReleaseWeak();
    cb_ = nullptr;
  }

  /**
   * @brief Swaps the contents with another WeakPtr.
   *
   * @param other The WeakPtr to swap with.
   */
  void Swap(WeakPtr &other) noexcept { std::swap(cb_, other.cb_); }

  // ========================================================================
  // Observers
  // ========================================================================

  /**
   * @brief Returns the strong reference count.
   *
   * @return std::size_t Number of SharedPtr instances owning the object,
   *         or 0 if the object no longer exists.
   */
  std::size_t use_count() const noexcept {
    return (cb_ != nullptr) ? cb_->use_count() : 0;
  }

  /**
   * @brief Checks if the observed object still exists.
   *
   * @return true if the object has been destroyed.
   */
  bool Expired() const noexcept { return use_count() == 0; }

  /**
   * @brief Creates a SharedPtr that shares ownership of the managed object.
   *
   * @return SharedPtr<T> A shared pointer to the object, or an empty one
   *         if the object no longer exists.
   */
  SharedPtr<T> Lock() const noexcept {
    if (Expired()) {
      return SharedPtr<T>{};
    }
    element_type *ptr_ = static_cast<element_type *>(cb_->data_ptr());
    return SharedPtr<T>{ptr_, cb_};
  }
};

/**
 * @brief Swap for ADL.
 */
template <typename T>
inline void swap(WeakPtr<T> &lhs, WeakPtr<T> &rhs) noexcept {
  lhs.Swap(rhs);
}

// ========================================================================
// Non member Comparison operators
// ========================================================================

/**
 * @brief Compares two SharedPtr objects.
 *
 * @param lhs First SharedPtr.
 * @param rhs Second SharedPtr.
 * @return std::strong_ordering Three-way comparison result.
 */
template <typename T, typename U>
auto operator<=>(const SharedPtr<T> &lhs, const SharedPtr<U> &rhs) noexcept {
  return std::compare_three_way{}(lhs.Get(), rhs.Get());
}

/**
 * @brief Equality comparison.
 *
 * @param lhs First SharedPtr.
 * @param rhs Second SharedPtr.
 * @return true If both point to the same object.
 */
template <typename T, typename U>
bool operator==(const SharedPtr<T> &lhs, const SharedPtr<U> &rhs) noexcept {
  return lhs.Get() == rhs.Get();
}

/**
 * @brief Compares a SharedPtr with nullptr.
 *
 * @param ptr_ The SharedPtr.
 * @return true If the SharedPtr is empty.
 */
template <typename T>
bool operator==(const SharedPtr<T> &lhs, std::nullptr_t) noexcept {
  return !lhs;
}

/**
 * @brief Compares a SharedPtr with nullptr.
 *
 * @param ptr_ The SharedPtr.
 * @return std::strong_ordering Three-way comparison result.
 */
template <typename T>
auto operator<=>(const SharedPtr<T> &ptr, std::nullptr_t) noexcept {
  return std::compare_three_way{}(
      ptr.Get(), static_cast<SharedPtr<T>::element_type *>(nullptr));
}
} // namespace my::memory