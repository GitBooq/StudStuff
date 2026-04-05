/**
 * @file Shared_ptr.h
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

#include <cstddef>         // for nullptr_t, size_t, ptrdiff_t
#include <concepts>        // for move_constructible
#include <compare>         // for compare_three_way
#include <memory>          // for default_delete
#include <type_traits>     // for remove_extent_t
#include <utility>         // for exchange, swap
#include "control_block.h" // for Cb_base, Cb_regular

namespace my::memory
{
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
        std::is_convertible_v<From *, To *> &&
        requires { sizeof(From); };

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
        requires std::is_convertible_v<typename From::element_type*,
                                       typename To::element_type*>;
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
    class SharedPtrBase
    {
    protected:
        Cb_base *cb; ///< Pointer to the control block

        /**
         * @brief Increments the strong reference count.
         */
        void add_ref() noexcept
        {
            if (cb != nullptr)
                cb->add_ref();
        }

        /**
         * @brief Decrements the strong reference count.
         *
         * May destroy the managed object and control block
         * if the count reaches zero.
         */
        void release() noexcept
        {
            if (cb != nullptr)
                cb->release();
        }

        /**
         * @brief Releases the control block.
         *
         * Calls release() and sets cb to nullptr.
         */
        void reset() noexcept
        {
            release();
            cb = nullptr;
        }

    public:
        /**
         * @brief Constructs an empty SharedPtrBase.
         */
        constexpr SharedPtrBase() noexcept : cb(nullptr) {}

        /**
         * @brief Constructs a SharedPtrBase from a control block.
         *
         * @param b Pointer to the control block (may be nullptr).
         */
        explicit constexpr SharedPtrBase(Cb_base *b) noexcept : cb(b) {}

        /**
         * @brief Copy constructor.
         *
         * @param other The SharedPtrBase to copy.
         *
         * @details Increments the reference count of the managed object.
         */
        SharedPtrBase(const SharedPtrBase &other) noexcept : cb(other.cb)
        {
            add_ref();
        }

        /**
         * @brief Move constructor.
         *
         * @param other The SharedPtrBase to move from.
         *
         * @details The other object becomes empty.
         */
        SharedPtrBase(SharedPtrBase &&other) noexcept
            : cb{std::exchange(other.cb, nullptr)}
        {
        }

        /**
         * @brief Destructor.
         *
         * Decrements the reference count. If this was the last
         * shared pointer, the managed object is destroyed.
         */
        ~SharedPtrBase() noexcept
        {
            release();
        }

        /**
         * @brief Copy/move assignment operator.
         *
         * @param other The SharedPtrBase to assign from.
         * @return SharedPtrBase& Reference to this object.
         *
         * @details Uses copy-and-swap idiom for strong exception safety.
         */
        SharedPtrBase &operator=(SharedPtrBase other) noexcept
        {
            swap(other);
            return *this;
        }

        /**
         * @brief Swaps the control blocks with another SharedPtrBase.
         *
         * @param other The SharedPtrBase to swap with.
         */
        void swap(SharedPtrBase &other) noexcept
        {
            std::swap(cb, other.cb);
        }

        /**
         * @brief Returns the strong reference count.
         *
         * @return size_t Number of shared_ptrs owning the object,
         *         or 0 if empty.
         */
        size_t use_count() const noexcept
        {
            return (cb != nullptr) ? cb->use_count() : 0;
        }

        /**
         * @brief Checks if the shared pointer owns an object.
         *
         * @return true if the shared pointer is not empty.
         */
        explicit operator bool() const noexcept
        {
            return cb && cb->use_count() > 0;
        }
    };

    // SharedPtr - Primary dummy template
    template <typename T>
    class SharedPtr {
            static_assert(sizeof(T) == 0, 
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
    template <NotArray T>
    class SharedPtr<T> : public SharedPtrBase
    {
        /**
         * @brief All WeakPtr specializations have access to private members
         *
         * Required for WeakPtr::lock() to access private constructor
         */
        template <typename>
        friend class WeakPtr;

    private:
        T *ptr; ///< Stored pointer to the managed object

        /**
         * @brief Private constructor for WeakPtr::lock().
         *
         * @param p Pointer to the managed object.
         * @param block Control block to share.
         */
        SharedPtr(T *p, Cb_base *block) noexcept
            : SharedPtrBase{block}, ptr{p}
        {
            if (cb != nullptr)
                cb->add_ref();
        }

    public:
        // ========================================================================
        // Constructors and destructor
        // ========================================================================

        /**
         * @brief Constructs an empty SharedPtr.
         */
        constexpr SharedPtr() : SharedPtrBase(), ptr(nullptr) {}

        /**
         * @brief Constructs an empty SharedPtr from nullptr.
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
        explicit SharedPtr(T *p)
            : SharedPtrBase(p ? new Cb_regular<T>(p) : nullptr), ptr(p) {}

        /**
         * @brief Constructs a SharedPtr from nullptr and custom deleter.
         *
         * @param d Deleter.
         *
         * @throws std::bad_alloc If control block allocation fails.
         */
        template <std::move_constructible Deleter>
        SharedPtr(std::nullptr_t, Deleter&& d)
            : SharedPtrBase(new Cb_regular<T, std::decay_t<Deleter>>(nullptr, std::forward<Deleter>(d))) {}

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
        SharedPtr(T *p, Deleter&& d)
            : SharedPtrBase(p ? new Cb_regular<T, std::decay_t<Deleter>>(p, std::forward<Deleter>(d)) : nullptr), ptr(p)
        {
        }

        /**
         * @brief Copy constructor.
         *
         * @param other The SharedPtr to copy.
         *
         * @details Increments the reference count.
         */
        SharedPtr(const SharedPtr &other) noexcept
            : SharedPtrBase(other), ptr(other.ptr) {}

        /**
         * @brief Converting copy constructor.
         *
         * @tparam U Type of the other SharedPtr.
         * @param other The SharedPtr to copy.
         *
         * @details Enabled only if U* is convertible to T*.
         *          Supports polymorphic conversions (Derived* to Base*).
         */
        template <NotArray U>
        SharedPtr(const SharedPtr<U> &other) noexcept
            : SharedPtrBase(other), ptr(other.get())
        {
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
              ptr{std::exchange(other.ptr, nullptr)}
        {
        }

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
        SharedPtr &operator=(SharedPtr other) noexcept
        {
            swap(other);
            return *this;
        }

        // ========================================================================
        // Modifiers
        // ========================================================================

        /**
         * @brief Releases ownership of the managed object.
         *
         * Set ptr of managed object to nullptr.
         * If this was the last SharedPtr owning the object,
         * the object is destroyed.
         */
        void reset() noexcept
        {
            SharedPtrBase::reset();
            ptr = nullptr;
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
        template <ConvertibleAndComplete<T> Y>
        void reset(Y *p)
        {
            SharedPtr<T> tmp{p};
            swap(tmp);
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
        T &operator*() const noexcept { return *get(); }

        /**
         * @brief Accesses members of the managed object.
         *
         * @return T* Pointer to the managed object.
         * @warning Undefined behavior if the pointer is nullptr.
         */
        T *operator->() const noexcept { return get(); }

        /**
         * @brief Returns the stored pointer.
         *
         * @return T* The stored pointer (may be nullptr).
         */
        T *get() const noexcept { return ptr; }

        // ========================================================================
        // Modifiers
        // ========================================================================

        /**
         * @brief Swaps the contents with another SharedPtr.
         *
         * @param other The SharedPtr to swap with.
         */
        void swap(SharedPtr &other) noexcept
        {
            SharedPtrBase::swap(other);
            std::swap(ptr, other.ptr);
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
    template <Array T>
    class SharedPtr<T> : public SharedPtrBase
    {
        /**
         * @brief All WeakPtr specializations have access to private members
         *
         * Required for WeakPtr::lock() to access private constructor
         */
        template <typename>
        friend class WeakPtr;

        using element_type = std::remove_extent_t<T>;
        using default_deleter = std::default_delete<element_type[]>;

    private:
        element_type *ptr; ///< Pointer to the first element of the array

        /**
         * @brief Private constructor for WeakPtr::lock().
         *
         * @param p Pointer to the first element.
         * @param block Control block to share.
         */
        SharedPtr(element_type *p, Cb_base *block) noexcept
            : SharedPtrBase{block}, ptr{p}
        {
            if (cb != nullptr)
                cb->add_ref();
        }

    public:
        // ========================================================================
        // Constructors and destructor
        // ========================================================================

        /**
         * @brief Constructs an empty SharedPtr.
         */
        constexpr SharedPtr() noexcept : SharedPtrBase(), ptr(nullptr) {}

        /**
         * @brief Constructs an empty SharedPtr from nullptr.
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
        explicit SharedPtr(element_type *p)
            : SharedPtrBase(p ? new Cb_regular<element_type, default_deleter>(p, default_deleter{})
                              : nullptr),
              ptr(p) {}

        /**
         * @brief Constructs a SharedPtr from a nullptr and a custom deleter.
         *
         * @param d Deleter.
         *
         * @throws std::bad_alloc If control block allocation fails.
         */
        template <std::move_constructible Deleter>
        SharedPtr(std::nullptr_t, Deleter&& d)
            : SharedPtrBase(new Cb_regular<element_type, std::decay_t<Deleter>>(nullptr, std::forward<Deleter>(d))) {}

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
        SharedPtr(element_type *p, Deleter&& d)
            : SharedPtrBase(p ? new Cb_regular<element_type, std::decay_t<Deleter>>(p, std::forward<Deleter>(d)) : nullptr), ptr(p)
        {
        }

        /**
         * @brief Copy constructor.
         *
         * @param other The SharedPtr to copy.
         */
        SharedPtr(const SharedPtr &other) noexcept
            : SharedPtrBase(other), ptr(other.ptr) {}

        /**
         * @brief Converting copy constructor.
         *
         * @tparam U Type of the other SharedPtr.
         * @param other The SharedPtr to copy.
         */
        template <Array U>
        SharedPtr(const SharedPtr<U> &other) noexcept
            : SharedPtrBase(other), ptr(other.get())
        {
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
              ptr{std::exchange(other.ptr, nullptr)}
        {
        }

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
        SharedPtr &operator=(SharedPtr other) noexcept
        {
            swap(other);
            return *this;
        }

        // ========================================================================
        // Modifiers
        // ========================================================================

        /**
         * @brief Releases ownership of the managed array.
         */
        void reset() noexcept
        {
            SharedPtrBase::reset();
            ptr = nullptr;
        }

        /**
         * @brief Replaces the managed array with a new pointer.
         *
         * @tparam Y Type of the new pointer.
         * @param p New raw pointer to manage.
         */
        template <typename Y>
        requires std::is_convertible_v<Y*, element_type*>
        void reset(Y *p)
        {
            SharedPtr<T> tmp{p}; // can throw
            swap(tmp);
        }

        // ========================================================================
        // Observers
        // ========================================================================

        /**
         * @brief Returns the stored pointer.
         *
         * @return T* Pointer to the first element (may be nullptr).
         */
        element_type *get() const noexcept { return ptr; }

        /**
         * @brief Accesses an element of the array.
         *
         * @param idx Index of the element.
         * @return T& Reference to the element.
         * @warning Undefined behavior if idx is out of bounds.
         */
        element_type &operator[](std::ptrdiff_t idx) const
        {
            return get()[idx];
        }

        // ========================================================================
        // Modifiers
        // ========================================================================

        /**
         * @brief Swaps the contents with another SharedPtr.
         *
         * @param other The SharedPtr to swap with.
         */
        void swap(SharedPtr &other) noexcept
        {
            SharedPtrBase::swap(other);
            std::swap(ptr, other.ptr);
        }
    };

    /**
     * @brief ADL Swap for all specializations
     */
    template <typename T>
    inline void swap(SharedPtr<T> &a, SharedPtr<T> &b) noexcept
    {
        a.swap(b);
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
    template <typename T>
    class WeakPtr
    {
        template <typename>
        friend class WeakPtr;

        // Underlying type for arrays
        using element_type = std::remove_extent_t<T>;

    private:
        Cb_base *cb; ///< Pointer to the control block

        /**
         * @brief Increments the weak reference count.
         */
        void add_weak() noexcept
        {
            if (cb != nullptr)
                cb->add_weak();
        }

        /**
         * @brief Decrements the weak reference count.
         */
        void release_weak() noexcept
        {
            if (cb != nullptr)
                cb->release_weak();
        }

    public:
        // ========================================================================
        // Constructors and destructor
        // ========================================================================

        /**
         * @brief Constructs an empty WeakPtr.
         */
        constexpr WeakPtr() noexcept : cb{nullptr} {}

        /**
         * @brief Constructs an empty WeakPtr from nullptr.
         */
        constexpr WeakPtr(std::nullptr_t) noexcept : WeakPtr() {}

        /**
         * @brief Copy constructor.
         *
         * @param other The WeakPtr to copy.
         */
        WeakPtr(const WeakPtr &other) noexcept : cb{other.cb}
        {
            add_weak();
        }

        /**
         * @brief Converting copy constructor.
         *
         * @tparam U Type of the other WeakPtr.
         * @param other The WeakPtr to copy.
         */
        template <typename U>
        WeakPtr(const WeakPtr<U> &other) noexcept : cb{other.cb}
        {
            using U_element = std::remove_extent_t<U>;
            static_assert(std::is_convertible_v<U_element *, element_type *>,
                          "Element types must be convertible");
            add_weak();
        }

        /**
         * @brief Constructs a WeakPtr from a SharedPtr.
         *
         * @tparam U Type of the shared pointer.
         * @param shared The SharedPtr to observe.
         */
        template <typename U>
        requires (std::is_array_v<T> == std::is_array_v<U>)
        WeakPtr(const SharedPtr<U> &shared) noexcept : cb{shared.cb}
        {
            using U_element = std::remove_extent_t<U>;
            static_assert(std::is_convertible_v<U_element *, element_type *>,
                          "Element types must be convertible");
            add_weak();
        }

        /**
         * @brief Move constructor.
         *
         * @param other The WeakPtr to move from.
         */
        WeakPtr(WeakPtr &&other) noexcept : cb{std::exchange(other.cb, nullptr)} {}

        /**
         * @brief Converting move constructor.
         *
         * @tparam U Type of the other WeakPtr.
         * @param other The WeakPtr to move from.
         */
        template <typename U>
        requires (std::is_array_v<T> == std::is_array_v<U>)
        WeakPtr(WeakPtr<U> &&other) noexcept : cb{std::exchange(other.cb, nullptr)}
        {
            using U_element = std::remove_extent_t<U>;
            static_assert(std::is_convertible_v<U_element *, element_type *>,
                          "Element types must be convertible");
        }

        /**
         * @brief Destructor.
         *
         * Decrements the weak reference count. Does not affect the managed object.
         */
        ~WeakPtr() noexcept
        {
            release_weak();
        }

        // ========================================================================
        // Assignment
        // ========================================================================

        /**
         * @brief Copy assignment operator.
         *
         * @param other The WeakPtr to assign from.
         * @return WeakPtr& Reference to this object.
         */
        WeakPtr &operator=(WeakPtr other) noexcept
        {
            swap(other);
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
        requires (std::is_array_v<T> == std::is_array_v<U>)
        WeakPtr &operator=(WeakPtr<U> other) noexcept
        {
            using U_element = std::remove_extent_t<U>;
            static_assert(std::is_convertible_v<U_element *, element_type *>,
                          "Element types must be convertible");
            swap(other);
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
        requires (std::is_array_v<T> == std::is_array_v<U>)
        WeakPtr &operator=(const SharedPtr<U> &shared) noexcept
        {
            using U_element = std::remove_extent_t<U>;
            static_assert(std::is_convertible_v<U_element *, element_type *>,
                          "Element types must be convertible");
            WeakPtr temp(shared);
            swap(temp);
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
        void reset() noexcept
        {
            release_weak();
            cb = nullptr;
        }

        /**
         * @brief Swaps the contents with another WeakPtr.
         *
         * @param other The WeakPtr to swap with.
         */
        void swap(WeakPtr &other) noexcept
        {
            std::swap(cb, other.cb);
        }

        // ========================================================================
        // Observers
        // ========================================================================

        /**
         * @brief Returns the strong reference count.
         *
         * @return std::size_t Number of SharedPtr instances owning the object,
         *         or 0 if the object no longer exists.
         */
        std::size_t use_count() const noexcept
        {
            return (cb != nullptr) ? cb->use_count() : 0;
        }

        /**
         * @brief Checks if the observed object still exists.
         *
         * @return true if the object has been destroyed.
         */
        bool expired() const noexcept
        {
            return use_count() == 0;
        }

        /**
         * @brief Creates a SharedPtr that shares ownership of the managed object.
         *
         * @return SharedPtr<T> A shared pointer to the object, or an empty one
         *         if the object no longer exists.
         */
        SharedPtr<T> lock() const noexcept
        {
            if (expired())
            {
                return SharedPtr<T>{};
            }
            else
            {
                element_type *ptr = static_cast<element_type *>(cb->get_data_ptr());
                return SharedPtr<T>{ptr, cb};
            }
        }
    };

    /**
     * @brief Swap for ADL.
     */
    template <typename T>
    inline void swap(WeakPtr<T> &lhs, WeakPtr<T> &rhs) noexcept
    {
        lhs.swap(rhs);
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
    auto operator<=>(const SharedPtr<T> &lhs, const SharedPtr<U> &rhs) noexcept
    {
        return std::compare_three_way{}(lhs.get(), rhs.get());
    }

    /**
     * @brief Equality comparison.
     *
     * @param lhs First SharedPtr.
     * @param rhs Second SharedPtr.
     * @return true If both point to the same object.
     */
    template <typename T, typename U>
    bool operator==(const SharedPtr<T> &lhs, const SharedPtr<U> &rhs) noexcept
    {
        return lhs.get() == rhs.get();
    }

    /**
     * @brief Compares a SharedPtr with nullptr.
     *
     * @param ptr The SharedPtr.
     * @return true If the SharedPtr is empty.
     */
    template <typename T>
    bool operator==(const SharedPtr<T> &lhs, std::nullptr_t) noexcept
    {
        return !lhs;
    }

    /**
     * @brief Compares a SharedPtr with nullptr.
     *
     * @param ptr The SharedPtr.
     * @return std::strong_ordering Three-way comparison result.
     */
    template <typename T>
    auto operator<=>(const SharedPtr<T> &ptr, std::nullptr_t) noexcept
    {
        return std::compare_three_way{}(ptr.get(), static_cast<SharedPtr<T>::element_type *>(nullptr));
    }
} // namespace my::memory