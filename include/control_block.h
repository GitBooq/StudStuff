/**
 * @file control_block.h
 * @brief Reference counting control block for SharedPtr and WeakPtr.
 *
 * This file contains the base class for reference counting control blocks
 * and the default implementation with a custom deleter.
 */

#pragma once

#include <cstddef>
#include <memory>  // for default_delete
#include <tuple>

namespace my::memory {
/**
 * @brief Base class for reference counting control blocks.
 *
 * Manages shared and weak reference counts. Concrete implementations
 * handle different deleter types.
 */
class CbBase {
 private:
  std::size_t use_cnt_{1};   ///< Strong reference counter
  std::size_t weak_cnt_{0};  ///< Weak reference counter

  /**
   * @brief Prevents destruction of control block during object destruction.
   *
   * Without this flag, if Sptr_1 during its destruction causes deletion
   * of Sptr_2 which holds a weak_ptr to Sptr_1, that weak_ptr would try
   * to delete Sptr_1's control block while it's still needed.
   */
  bool is_destroying_{false};

 public:
  // ========================================================================
  // Virtual interface
  // ========================================================================

  /**
   * @brief Destroys the managed object.
   *
   * Called when the strong reference count reaches zero.
   * Implemented by derived classes to delete the managed object.
   *
   * @note Must be noexcept. The deleter should not throw.
   */
  virtual void DestroyObject() noexcept = 0;

  /**
   * @brief Destroys the control block itself.
   *
   * Called when both reference counts reach zero.
   * Implemented by derived classes to free the control block memory.
   *
   * @note Must be noexcept. Typically does `delete this`.
   */
  virtual void DestroyBlock() noexcept = 0;

  /**
   * @brief Returns a pointer to the managed object.
   *
   * Used by weak_ptr::lock() to obtain the object pointer.
   *
   * @return void* Raw pointer to the managed object (may be nullptr).
   */
  virtual void* data_ptr() const noexcept = 0;

  /**
   * @brief Virtual destructor.
   *
   * Required for proper cleanup of derived classes.
   */
  virtual ~CbBase() noexcept = default;

  // ========================================================================
  // Reference counting
  // ========================================================================

  /**
   * @brief Increments the strong reference count.
   */
  void AddRef() noexcept { ++use_cnt_; }

  /**
   * @brief Increments the weak reference count.
   */
  void AddWeak() noexcept { ++weak_cnt_; }

  /**
   * @brief Returns the strong reference count.
   *
   * @return std::size_t Number of SharedPtrs owning the object.
   */
  std::size_t use_count() const noexcept { return use_cnt_; }

  /**
   * @brief Returns the weak reference count.
   *
   * @return std::size_t Number of weak_ptrs observing the object.
   */
  std::size_t weak_count() const noexcept { return weak_cnt_; }

  // ========================================================================
  // Resource management
  // ========================================================================

  /**
   * @brief Decrements the strong reference count.
   *
   * When the count reaches zero:
   * - Marks the block as destroying(by setting flag)
   * - Calls destroy_object() to delete the managed object
   * - If no weak references exist, destroys the control block
   */
  void Release() noexcept {
    if (--use_cnt_ == 0) {
      is_destroying_ = true;
      DestroyObject();
      is_destroying_ = false;
      if (weak_cnt_ == 0) {
        DestroyBlock();
      }
    }
  }

  /**
   * @brief Decrements the weak reference count.
   *
   * When the count reaches zero and no strong references exist,
   * destroys the control block.
   *
   * @note Does not destroy the block if object destruction is in progress.
   */
  void ReleaseWeak() noexcept {
    if (--weak_cnt_ == 0 && use_cnt_ == 0 && !is_destroying_) {
      DestroyBlock();
    }
  }
};

// ========================================================================
// Regular control block with custom deleter
// ========================================================================

/**
 * @brief Control block that stores a pointer to managed object and a deleter.
 *
 * Uses EBCO via std::tuple to avoid memory overhead for empty deleters.
 *
 * @tparam T Type of the managed object.
 * @tparam Deleter Type of the deleter functor (default:
 * std::default_delete<T>).
 */
template <typename T, typename Deleter = std::default_delete<T>>
class CbRegular final : public CbBase {
 private:
  T* ptr_;                           ///< Pointer to the managed object
  std::tuple<Deleter> deleter_tup_;  ///< Deleter (EBCO optimized)

 public:
  /**
   * @brief Constructs a control block with a deleter.
   *
   * @param p Raw pointer to the managed object.
   * @param d Deleter functor (default constructed if omitted).
   */
  explicit CbRegular(T* p, Deleter d = Deleter{})
      : ptr_{p}, deleter_tup_(std::move(d)) {}

  /**
   * @brief Destroys the managed object using the stored deleter.
   *
   * The pointer is set to nullptr after deletion.
   */
  void DestroyObject() noexcept override {
    auto& [del] = deleter_tup_;
    del(ptr_);
    ptr_ = nullptr;
  }

  /**
   * @brief Destroys the control block itself.
   */
  void DestroyBlock() noexcept override { delete this; }

  /**
   * @brief Returns a pointer to the managed object.
   *
   * @return void* Raw pointer (may be null).
   */
  void* data_ptr() const noexcept override { return ptr_; }

  /**
   * @brief Destructor.
   *
   * @note The managed object is destroyed in destroy_object(),
   *       not in this destructor.
   */
  ~CbRegular() noexcept = default;
};
}  // namespace my::memory