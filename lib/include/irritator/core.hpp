// Copyright (c) 2020 INRA Distributed under the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ORG_VLEPROJECT_IRRITATOR_2020
#define ORG_VLEPROJECT_IRRITATOR_2020

#include <algorithm>
#include <limits>

#ifdef __has_include
#if __has_include(<numbers>)
#include <numbers>
#define irt_have_numbers 1
#else
#define irt_have_numbers 0
#endif
#endif

#include <string_view>
#include <type_traits>

#include <cmath>
#include <cstdint>
#include <cstring>

/*****************************************************************************
 *
 * Helper macros
 *
 ****************************************************************************/

#ifndef irt_assert
#include <cassert>
#define irt_assert(_expr) assert(_expr)
#endif

namespace irt {

static inline bool is_fatal_breakpoint = true;

}

#ifndef NDEBUG
#if (defined(__i386__) || defined(__x86_64__)) && defined(__GNUC__) &&         \
  __GNUC__ >= 2
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            __asm__ __volatile__("int $03");                                   \
    } while (0)
#elif (defined(_MSC_VER) || defined(__DMC__)) && defined(_M_IX86)
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            __asm int 3h                                                       \
    } while (0)
#elif defined(_MSC_VER)
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            __debugbreak();                                                    \
    } while (0)
#elif defined(__alpha__) && !defined(__osf__) && defined(__GNUC__) &&          \
  __GNUC__ >= 2
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            __asm__ __volatile__("bpt");                                       \
    } while (0)
#elif defined(__APPLE__)
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            __builtin_trap();                                                  \
    } while (0)
#else /* !__i386__ && !__alpha__ */
#define irt_breakpoint()                                                       \
    do {                                                                       \
        if (::irt::is_fatal_breakpoint)                                        \
            raise(SIGTRAP);                                                    \
    } while (0)
#endif /* __i386__ */
#else
#define irt_breakpoint()                                                       \
    do {                                                                       \
    } while (0)
#endif

#define irt_bad_return(status__)                                               \
    do {                                                                       \
        irt_breakpoint();                                                      \
        return status__;                                                       \
    } while (0)

#define irt_return_if_bad(expr__)                                              \
    do {                                                                       \
        auto status__ = (expr__);                                              \
        if (status__ != status::success) {                                     \
            irt_breakpoint();                                                  \
            return status__;                                                   \
        }                                                                      \
    } while (0)

#define irt_return_if_fail(expr__, status__)                                   \
    do {                                                                       \
        if (!(expr__)) {                                                       \
            irt_breakpoint();                                                  \
            return status__;                                                   \
        }                                                                      \
    } while (0)

#if defined(__GNUC__)
#define irt_unreachable() __builtin_unreachable();
#elif defined(_MSC_VER)
#define irt_unreachable() __assume(0)
#else
#define irt_unreachable()
#endif

namespace irt {

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using sz  = size_t;
using f32 = float;
using f64 = double;

#ifdef IRRITATOR_REAL_TYPE_F64
using real = f64;
#else
using real = f32;
#endif //  IRRITATOR_REAL_TYPE

//! @brief An helper function to initialize floating point number and disable
//! warnings the IRRITATOR_REAL_TYPE_F64 is defined.
//!
//! @param v The floating point number to convert to float, double or long
//! double.
//! @return A real.
inline constexpr real to_real(long double v) noexcept
{
    return static_cast<real>(v);
}

namespace literals {

//! @brief An helper literal function to initialize floating point number and
//! disable warnings the IRRITATOR_REAL_TYPE_F64 is defined.
//!
//! @param v The floating point number to convert to float, double or long
//! double.
//! @return A real.
inline constexpr real operator"" _r(long double v) noexcept
{
    return static_cast<real>(v);
}

} // namespace literals

constexpr static inline real one   = to_real(1.);
constexpr static inline real two   = to_real(2.);
constexpr static inline real three = to_real(3.);
constexpr static inline real four  = to_real(4.);
constexpr static inline real zero  = to_real(0.);

inline u16 make_halfword(u8 a, u8 b) noexcept
{
    return static_cast<u16>((a << 8) | b);
}

inline void unpack_halfword(u16 halfword, u8* a, u8* b) noexcept
{
    *a = static_cast<u8>((halfword >> 8) & 0xff);
    *b = static_cast<u8>(halfword & 0xff);
}

inline u32 make_word(u16 a, u16 b) noexcept
{
    return (static_cast<u32>(a) << 16) | static_cast<u32>(b);
}

inline void unpack_word(u32 word, u16* a, u16* b) noexcept
{
    *a = static_cast<u16>((word >> 16) & 0xffff);
    *b = static_cast<u16>(word & 0xffff);
}

inline u64 make_doubleword(u32 a, u32 b) noexcept
{
    return (static_cast<u64>(a) << 32) | static_cast<u64>(b);
}

inline void unpack_doubleword(u64 doubleword, u32* a, u32* b) noexcept
{
    *a = static_cast<u32>((doubleword >> 32) & 0xffffffff);
    *b = static_cast<u32>(doubleword & 0xffffffff);
}

inline u32 unpack_doubleword_left(u64 doubleword) noexcept
{
    return static_cast<u32>((doubleword >> 32) & 0xffffffff);
}

inline u32 unpack_doubleword_right(u64 doubleword) noexcept
{
    return static_cast<u32>(doubleword & 0xffffffff);
}

template<typename Integer>
constexpr typename std::make_unsigned<Integer>::type to_unsigned(Integer value)
{
    irt_assert(value >= 0);
    return static_cast<typename std::make_unsigned<Integer>::type>(value);
}

template<class C>
constexpr int length(const C& c) noexcept
{
    return static_cast<int>(c.size());
}

template<class T, size_t N>
constexpr int length(const T (&array)[N]) noexcept
{
    (void)array;

    return static_cast<int>(N);
}

template<typename Identifier>
constexpr Identifier undefined() noexcept
{
    static_assert(
      std::is_enum<Identifier>::value,
      "Identifier must be a enumeration: enum class id : unsigned {};");

    return static_cast<Identifier>(0);
}

template<typename Identifier>
constexpr bool is_undefined(Identifier id) noexcept
{
    static_assert(
      std::is_enum<Identifier>::value,
      "Identifier must be a enumeration: enum class id : unsigned {};");

    return id == undefined<Identifier>();
}

template<typename Identifier>
constexpr bool is_defined(Identifier id) noexcept
{
    static_assert(
      std::is_enum<Identifier>::value,
      "Identifier must be a enumeration: enum class id : unsigned {};");

    return id != undefined<Identifier>();
}

//! @brief A simple enumeration to integral helper function.
//! @tparam Enum An enumeration
//! @tparam Integer The underlying_type deduce from @c Enum
//! @param e The element in enumeration to convert.
//! @return An integral.
template<class Enum, class Integer = typename std::underlying_type<Enum>::type>
constexpr Integer ordinal(Enum e) noexcept
{
    static_assert(std::is_enum<Enum>::value,
                  "Identifier must be a enumeration");
    return static_cast<Integer>(e);
}

//! @brief A simpole integral to enumeration helper function.
//! @tparam Enum An enumeration
//! @tparam Integer The underlying_type deduce from @c Enum
//! @param i The integral to convert.
//! @return A element un enumeration.
template<class Enum, class Integer = typename std::underlying_type<Enum>::type>
constexpr Enum enum_cast(Integer i) noexcept
{
    static_assert(std::is_enum<Enum>::value,
                  "Identifier must be a enumeration");
    return static_cast<Enum>(i);
}

//! @brief returns an iterator to the result or end if not found
//!
//! Binary search function which returns an iterator to the result or end if
//! not found using the lower_bound standard function.
template<typename Iterator, typename T>
Iterator binary_find(Iterator begin, Iterator end, const T& value)
{
    begin = std::lower_bound(begin, end, value);
    return (!(begin == end) && (value == *begin)) ? begin : end;
}

//! @brief returns an iterator to the result or end if not found
//!
//! Binary search function which returns an iterator to the result or end if
//! not found using the lower_bound standard function.
template<typename Iterator, typename T, typename Compare>
Iterator binary_find(Iterator begin, Iterator end, const T& value, Compare comp)
{
    begin = std::lower_bound(begin, end, value, comp);
    return (!(begin == end) && (value == *begin)) ? begin : end;
}

/*****************************************************************************
 *
 * Return status of many function
 *
 ****************************************************************************/

enum class status
{
    success,
    unknown_dynamics,
    block_allocator_bad_capacity,
    block_allocator_not_enough_memory,
    head_allocator_bad_capacity,
    head_allocator_not_enough_memory,
    simulation_not_enough_model,
    simulation_not_enough_message,
    simulation_not_enough_connection,
    vector_init_capacity_error,
    vector_not_enough_memory,
    data_array_init_capacity_error,
    data_array_not_enough_memory,
    source_unknown,
    source_empty,
    model_connect_output_port_unknown,
    model_connect_already_exist,
    model_connect_bad_dynamics,
    model_queue_bad_ta,
    model_queue_full,
    model_dynamic_queue_source_is_null,
    model_dynamic_queue_full,
    model_priority_queue_source_is_null,
    model_priority_queue_full,
    model_integrator_dq_error,
    model_integrator_X_error,
    model_integrator_internal_error,
    model_integrator_output_error,
    model_integrator_running_without_x_dot,
    model_integrator_ta_with_bad_x_dot,
    model_quantifier_bad_quantum_parameter,
    model_quantifier_bad_archive_length_parameter,
    model_quantifier_shifting_value_neg,
    model_quantifier_shifting_value_less_1,
    model_time_func_bad_init_message,
    model_flow_bad_samplerate,
    model_flow_bad_data,
    gui_not_enough_memory,
    io_not_enough_memory,
    io_file_format_error,
    io_file_format_source_number_error,
    io_file_source_full,
    io_file_format_model_error,
    io_file_format_model_number_error,
    io_file_format_model_unknown,
    io_file_format_dynamics_unknown,
    io_file_format_dynamics_limit_reach,
    io_file_format_dynamics_init_error,
    filter_threshold_condition_not_satisfied
};

constexpr i8 status_last() noexcept
{
    return static_cast<i8>(status::io_file_format_dynamics_init_error);
}

constexpr sz status_size() noexcept
{
    return static_cast<sz>(status_last() + static_cast<i8>(1));
}

constexpr bool is_success(status s) noexcept { return s == status::success; }

constexpr bool is_bad(status s) noexcept { return s != status::success; }

template<typename... Args>
constexpr bool is_status_equal(status s, Args... args) noexcept
{
    return ((s == args) || ... || false);
}

inline status check_return(status s) noexcept
{
    if (s != status::success)
        irt_breakpoint();

    return s;
}

template<typename T, typename... Args>
constexpr bool match(const T& s, Args... args) noexcept
{
    return ((s == args) || ... || false);
}

template<class T, class... Rest>
constexpr bool are_all_same() noexcept
{
    return (std::is_same_v<T, Rest> && ...);
}

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp)
{
    return std::fabs(x - y) <=
             std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp ||
           std::fabs(x - y) < std::numeric_limits<T>::min();
}

/*****************************************************************************
 *
 * Definition of a lightweight std::function
 *
 ****************************************************************************/

template<class F>
class function_ref;

template<class R, class... Args>
class function_ref<R(Args...)>
{
public:
    constexpr function_ref() noexcept = default;

    //! Creates a `function_ref` which refers to the same callable as `rhs`.
    constexpr function_ref(const function_ref<R(Args...)>& rhs) noexcept =
      default;

    //! Constructs a `function_ref` referring to `f`.
    //!
    //! \synopsis template <typename F> constexpr function_ref(F &&f) noexcept
    template<
      typename F,
      std::enable_if_t<!std::is_same<std::decay_t<F>, function_ref>::value &&
                       std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr function_ref(F&& f) noexcept
      : obj(const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f))))
    {
        cb = [](void* obj, Args... args) -> R {
            return std::invoke(
              *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
              std::forward<Args>(args)...);
        };
    }

    //! Makes `*this` refer to the same callable as `rhs`.
    constexpr function_ref<R(Args...)>& operator    =(
      const function_ref<R(Args...)>& rhs) noexcept = default;

    //! Makes `*this` refer to `f`.
    //!
    //! \synopsis template <typename F> constexpr function_ref &operator=(F &&f)
    //! noexcept;
    template<
      typename F,
      std::enable_if_t<std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr function_ref<R(Args...)>& operator=(F&& f) noexcept
    {
        obj = reinterpret_cast<void*>(std::addressof(f));
        cb  = [](void* obj, Args... args) {
            return std::invoke(
              *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
              std::forward<Args>(args)...);
        };

        return *this;
    }

    constexpr void swap(function_ref<R(Args...)>& rhs) noexcept
    {
        std::swap(obj, rhs.obj);
        std::swap(cb, rhs.cb);
    }

    R operator()(Args... args) const
    {
        return cb(obj, std::forward<Args>(args)...);
    }

    constexpr bool empty() const noexcept { return obj == nullptr; }

    constexpr void reset() noexcept
    {
        obj = nullptr;
        cb  = nullptr;
    }

private:
    void* obj               = nullptr;
    R (*cb)(void*, Args...) = nullptr;
};

//! Swaps the referred callables of `lhs` and `rhs`.
template<typename R, typename... Args>
constexpr void swap(function_ref<R(Args...)>& lhs,
                    function_ref<R(Args...)>& rhs) noexcept
{
    lhs.swap(rhs);
}

template<typename R, typename... Args>
function_ref(R (*)(Args...)) -> function_ref<R(Args...)>;

template<typename R, typename... Args>
function_ref(R (*)(Args...) noexcept) -> function_ref<R(Args...) noexcept>;

/*****************************************************************************
 *
 * Allocator
 *
 ****************************************************************************/

using global_alloc_function_type = function_ref<void*(sz size)>;
using global_free_function_type  = function_ref<void(void* ptr)>;

static inline void* malloc_wrapper(sz size) { return std::malloc(size); }

static inline void free_wrapper(void* ptr)
{
    if (ptr)
        std::free(ptr);
}

static inline global_alloc_function_type g_alloc_fn{ malloc_wrapper };
static inline global_free_function_type  g_free_fn{ free_wrapper };

/*****************************************************************************
 *
 * Definition of Time
 *
 * TODO:
 * - enable template definition of float or [float|double,bool absolute]
 *   representation of time?
 *
 ****************************************************************************/

using time = real;

template<typename T>
struct time_domain
{};

template<>
struct time_domain<time>
{
    using time_type = time;

    static constexpr const real infinity =
      std::numeric_limits<real>::infinity();
    static constexpr const real negative_infinity =
      -std::numeric_limits<real>::infinity();
    static constexpr const real zero = 0;

    static constexpr bool is_infinity(time t) noexcept
    {
        return t == infinity || t == negative_infinity;
    }

    static constexpr bool is_zero(time t) noexcept { return t == zero; }
};

/*****************************************************************************
 *
 * Containers
 *
 ****************************************************************************/

//! @brief Compute the best size to fit the small storage size.
template<size_t N>
using small_storage_size_t = std::conditional_t<
  (N < std::numeric_limits<uint8_t>::max()),
  uint8_t,
  std::conditional_t<
    (N < std::numeric_limits<uint16_t>::max()),
    uint16_t,
    std::conditional_t<
      (N < std::numeric_limits<uint32_t>::max()),
      uint32_t,
      std::conditional_t<(N < std::numeric_limits<uint32_t>::max()),
                         uint64_t,
                         size_t>>>>;

//! @brief A vector like class with dynamic allocation.
//! @tparam T Any type (trivial or not).
template<typename T>
class vector
{
    static_assert(std::is_copy_assignable_v<T> ||
                  std::is_copy_constructible_v<T> ||
                  std::is_nothrow_constructible_v<T> ||
                  std::is_move_assignable_v<T> ||
                  std::is_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<T>);

    T*  m_data     = nullptr;
    i32 m_size     = 0;
    i32 m_capacity = 0;

public:
    using size_type       = sz;
    using index_type      = i32;
    using iterator        = T*;
    using const_iterator  = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;

    constexpr vector() noexcept = default;
    vector(i32 capacity) noexcept;
    vector(i32 capacity, i32 size) noexcept;
    ~vector() noexcept;

    constexpr vector(const vector& other) noexcept;
    constexpr vector& operator=(const vector& other) noexcept;
    constexpr vector(vector&& other) noexcept;
    constexpr vector& operator=(vector&& other) noexcept;

    void resize(i32 size) noexcept;
    void reserve(i32 new_capacity) noexcept;

    void destroy() noexcept; // clear all elements and free memory (size = 0,
                             // capacity = 0 after).

    constexpr void clear() noexcept; // clear all elements (size = 0 after).

    constexpr T*       data() noexcept;
    constexpr const T* data() const noexcept;

    constexpr reference       front() noexcept;
    constexpr const_reference front() const noexcept;
    constexpr reference       back() noexcept;
    constexpr const_reference back() const noexcept;

    constexpr reference       operator[](const index_type index) noexcept;
    constexpr const_reference operator[](const index_type index) const noexcept;

    constexpr iterator       begin() noexcept;
    constexpr const_iterator begin() const noexcept;
    constexpr iterator       end() noexcept;
    constexpr const_iterator end() const noexcept;

    constexpr bool can_alloc(int number = 1) const noexcept;
    constexpr sz   size() const noexcept;
    constexpr i32  ssize() const noexcept;
    constexpr sz   capacity() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr bool full() const noexcept;

    template<typename... Args>
    constexpr reference emplace_back(Args&&... args) noexcept;

    constexpr i32 find(const T& t) const noexcept;

    constexpr void pop_back() noexcept;
    constexpr void swap_pop_back(index_type index) noexcept;

    constexpr void erase(iterator it) noexcept;
    constexpr void erase(iterator begin, iterator end) noexcept;

private:
    i32 compute_new_capacity(i32 new_capacity) const;
};

//! @brief A small_string without heap allocation.
template<sz length = 8>
class small_string
{
public:
    static_assert(length >= 2);

    using size_type = small_storage_size_t<length>;

private:
    char      m_buffer[length];
    size_type m_size;

public:
    using iterator        = char*;
    using const_iterator  = const char*;
    using reference       = char&;
    using const_reference = const char&;

    constexpr small_string() noexcept;
    constexpr small_string(const small_string& str) noexcept;
    constexpr small_string(small_string&& str) noexcept;
    constexpr small_string(const char* str) noexcept;
    constexpr small_string(const std::string_view str) noexcept;

    constexpr small_string& operator=(const small_string& str) noexcept;
    constexpr small_string& operator=(small_string&& str) noexcept;
    constexpr small_string& operator=(const char* str) noexcept;
    constexpr small_string& operator=(const std::string_view str) noexcept;

    constexpr void assign(const std::string_view str) noexcept;
    constexpr void clear() noexcept;
    void           resize(size_type size) noexcept;
    constexpr bool empty() const noexcept;

    constexpr sz size() const noexcept;
    constexpr sz capacity() const noexcept;

    constexpr reference       operator[](const size_type index) noexcept;
    constexpr const_reference operator[](const size_type index) const noexcept;

    constexpr std::string_view sv() const noexcept;
    constexpr const char*      c_str() const noexcept;

    constexpr iterator       begin() noexcept;
    constexpr iterator       end() noexcept;
    constexpr const_iterator begin() const noexcept;
    constexpr const_iterator end() const noexcept;

    constexpr bool operator==(const small_string& rhs) const noexcept;
    constexpr bool operator!=(const small_string& rhs) const noexcept;
    constexpr bool operator>(const small_string& rhs) const noexcept;
    constexpr bool operator<(const small_string& rhs) const noexcept;
    constexpr bool operator==(const char* rhs) const noexcept;
    constexpr bool operator!=(const char* rhs) const noexcept;
    constexpr bool operator>(const char* rhs) const noexcept;
    constexpr bool operator<(const char* rhs) const noexcept;
};

//! @brief A small array of reals.
//!
//! This class in mainly used to store message and observation in the simulation
//! kernel.
template<size_t length>
struct fixed_real_array
{
    using value_type      = real;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static_assert(length >= 1, "fixed_real_array length must be >= 1");

    real data[length]{};

    constexpr size_type size() const noexcept
    {
        for (size_t i = length; i != 0u; --i)
            if (data[i - 1u])
                return i;

        return 0u;
    }

    constexpr difference_type ssize() const noexcept
    {
        return static_cast<difference_type>(size());
    }

    constexpr fixed_real_array() noexcept = default;

    template<typename... Args>
    constexpr fixed_real_array(Args&&... args)
      : data{ std::forward<Args>(args)... }
    {
        auto size = sizeof...(args);
        for (; size < length; ++size)
            data[size] = zero;
    }

    template<typename U,
             typename = std::enable_if_t<std::is_convertible_v<U, irt::real>>>
    constexpr fixed_real_array(std::initializer_list<U> ilist) noexcept
    {
        irt_assert(ilist.size() <= length);

        int index = 0;
        for (const U& element : ilist) {
            data[index++] = element;
        }
    }

    constexpr real operator[](const difference_type i) const noexcept
    {
        return data[i];
    }

    constexpr real& operator[](const difference_type i) noexcept
    {
        return data[i];
    }

    constexpr void reset() noexcept { std::fill_n(data, length, zero); }
};

using message             = fixed_real_array<3>;
using dated_message       = fixed_real_array<4>;
using observation_message = fixed_real_array<4>;

/*****************************************************************************
 *
 * Flat list
 *
 ****************************************************************************/

template<typename T>
class block_allocator
{
public:
    using value_type = T;

    union block
    {
        block*                                                     next;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    };

private:
    block* blocks{ nullptr };    // contains all preallocated blocks
    block* free_head{ nullptr }; // a free list
    sz     size{ 0 };            // number of active elements allocated
    sz     max_size{ 0 }; // number of elements allocated (with free_head)
    sz     capacity{ 0 }; // capacity of the allocator

public:
    block_allocator() = default;

    block_allocator(const block_allocator&) = delete;
    block_allocator& operator=(const block_allocator&) = delete;

    ~block_allocator() noexcept
    {
        if (blocks)
            g_free_fn(blocks);
    }

    status init(sz new_capacity) noexcept
    {
        if (new_capacity == 0)
            return status::block_allocator_bad_capacity;

        if (new_capacity != capacity) {
            if (blocks)
                g_free_fn(blocks);

            blocks =
              static_cast<block*>(g_alloc_fn(new_capacity * sizeof(block)));
            if (blocks == nullptr)
                return status::block_allocator_not_enough_memory;
        }

        size      = 0;
        max_size  = 0;
        capacity  = new_capacity;
        free_head = nullptr;

        return status::success;
    }

    void reset() noexcept
    {
        if (capacity > 0) {
            size      = 0;
            max_size  = 0;
            free_head = nullptr;
        }
    }

    T* alloc() noexcept
    {
        block* new_block = nullptr;

        if (free_head != nullptr) {
            new_block = free_head;
            free_head = free_head->next;
        } else {
            irt_assert(max_size < capacity);
            new_block = reinterpret_cast<block*>(&blocks[max_size++]);
        }
        ++size;

        return reinterpret_cast<T*>(new_block);
    }

    u32 index(const T* ptr) const noexcept
    {
        irt_assert(ptr - reinterpret_cast<T*>(blocks) > 0);
        auto ptrdiff = ptr - reinterpret_cast<T*>(blocks);
        return static_cast<u32>(ptrdiff);
    }

    u32 alloc_index() noexcept
    {
        block* new_block = nullptr;

        if (free_head != nullptr) {
            new_block = free_head;
            free_head = free_head->next;
        } else {
            irt_assert(max_size < capacity);
            new_block = reinterpret_cast<block*>(&blocks[max_size++]);
        }
        ++size;

        irt_assert(new_block - blocks >= 0);
        return static_cast<u32>(new_block - blocks);
    }

    bool can_alloc() noexcept
    {
        return free_head != nullptr || max_size < capacity;
    }

    void free(T* n) noexcept
    {
        irt_assert(n);

        block* ptr = reinterpret_cast<block*>(n);

        ptr->next = free_head;
        free_head = ptr;

        --size;

        if (size == 0) {         // A special part: if it no longer exists
            max_size  = 0;       // we reset the free list and the number
            free_head = nullptr; // of elements allocated.
        }
    }

    void free(u32 index) noexcept
    {
        auto* to_free = reinterpret_cast<T*>(&(blocks[index]));
        free(to_free);
    }

    bool can_alloc(size_t number) const noexcept
    {
        return number + size < capacity;
    }

    value_type& operator[](u32 index) noexcept
    {
        return *reinterpret_cast<T*>(&(blocks[index]));
    }

    const value_type& operator[](u32 index) const noexcept
    {
        return *reinterpret_cast<T*>(&(blocks[index]));
    }
};

template<typename T>
struct list_view_node
{
    using value_type = T;

    T   value;
    u32 prev;
    u32 next;
};

template<typename T>
class list_view;

template<typename T>
class list_view_const;

template<typename ContainerType, typename T>
class list_view_iterator_impl
{
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value             = T;
    using pointer           = T*;
    using reference         = T&;
    using container_type    = ContainerType;

    template<typename X>
    friend class list_view;

    template<typename X>
    friend class list_view_const;

private:
    container_type& lst;
    u32             id;

public:
    list_view_iterator_impl(container_type& lst_, u32 id_) noexcept
      : lst(lst_)
      , id(id_)
    {}

    list_view_iterator_impl(const list_view_iterator_impl& other) noexcept
      : lst(other.lst)
      , id(other.id)
    {}

    list_view_iterator_impl& operator=(
      const list_view_iterator_impl& other) noexcept
    {
        list_view_iterator_impl tmp(other);

        irt_assert(&tmp.lst == &lst);
        std::swap(tmp.id, id);

        return *this;
    }

    reference operator*() const { return lst.m_allocator[id].value; }

    pointer operator->() { return &lst.m_allocator[id].value; }

    list_view_iterator_impl& operator++()
    {
        if (id == static_cast<u32>(-1)) {
            id = unpack_doubleword_left(lst.m_list);
        } else {
            id = lst.m_allocator[id].next;
        }

        return *this;
    }

    list_view_iterator_impl operator++(int)
    {
        list_view_iterator_impl tmp = *this;
        if (id == static_cast<u32>(-1)) {
            id = unpack_doubleword_left(lst.m_list);
        } else {
            id = lst.m_allocator[id].next;
        }
        return tmp;
    }

    list_view_iterator_impl& operator--()
    {
        if (id == static_cast<u32>(-1)) {
            id = unpack_doubleword_right(lst.m_list);
        } else {
            id = lst.m_allocator[id].prev;
        }
        return *this;
    }

    list_view_iterator_impl operator--(int)
    {
        list_view_iterator_impl tmp = *this;
        if (id == static_cast<u32>(-1)) {
            id = unpack_doubleword_right(lst.m_list);
        } else {
            id = lst.m_allocator[id].prev;
        }
        return tmp;
    }

    friend bool operator==(const list_view_iterator_impl& a,
                           const list_view_iterator_impl& b)
    {
        return &a.lst == &b.lst && a.id == b.id;
    }

    friend bool operator!=(const list_view_iterator_impl& a,
                           const list_view_iterator_impl& b)
    {
        return &a.lst != &b.lst || a.id != b.id;
    }
};

template<typename T>
class list_view
{
public:
    using node_type       = list_view_node<T>;
    using allocator_type  = block_allocator<node_type>;
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using this_type       = list_view<T>;
    using iterator        = list_view_iterator_impl<this_type, T>;
    using const_iterator  = list_view_iterator_impl<this_type, const T>;

    // friend class list_view_iterator_impl<this_type, T>;
    template<typename X, typename Y>
    friend class list_view_iterator_impl;

private:
    allocator_type& m_allocator;
    u64&            m_list;

public:
    list_view(allocator_type& allocator, u64& id) noexcept
      : m_allocator(allocator)
      , m_list(id)
    {}

    ~list_view() noexcept = default;

    void clear() noexcept
    {
        u32 begin = unpack_doubleword_left(m_list);
        while (begin != static_cast<u32>(-1)) {
            auto to_delete = begin;
            begin          = m_allocator[begin].next;
            m_allocator.free(to_delete);
        }

        m_list = static_cast<u64>(-1);
    }

    bool empty() const noexcept { return m_list == static_cast<u64>(-1); }

    iterator begin() noexcept
    {
        return iterator(*this, unpack_doubleword_left(m_list));
    }

    iterator end() noexcept { return iterator(*this, static_cast<u32>(-1)); }

    const_iterator begin() const noexcept
    {
        return const_iterator(*this, unpack_doubleword_left(m_list));
    }

    const_iterator end() const noexcept
    {
        return const_iterator(*this, static_cast<u32>(-1));
    }

    reference front() noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_left(m_list)].value;
    }

    const_reference front() const noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_left(m_list)].value;
    }

    reference back() noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_right(m_list)].value;
    }

    const_reference back() const noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_right(m_list)].value;
    }

    //! @brief Inserts a new element into the container directly before pos.
    template<typename... Args>
    iterator emplace(iterator pos, Args&&... args) noexcept
    {
        if (pos.id == static_cast<u32>(-1))
            return emplace_back(std::forward<Args>(args)...);

        if (m_allocator[pos.id].prev == static_cast<u32>(-1))
            return emplace_front(std::forward<Args>(args)...);

        u32 new_node = m_allocator.alloc_index();
        new (&m_allocator[new_node].value) T{ std::forward<Args>(args)... };

        m_allocator[new_node].prev = pos.id;
        m_allocator[new_node].next = m_allocator[pos.id].next;
        m_allocator[pos.id].next   = new_node;

        return iterator(*this, new_node);
    }

    //! @brief Erases the specified elements from the container.
    //!
    //! If pos refers to the last element, then the end() iterator is returned.
    //! If last==end() prior to removal, then the updated end() iterator is
    //! returned. If [first, last) is an empty range, then last is returned.
    //!
    //! @return iIterator following the last removed element.
    iterator erase(iterator pos) noexcept
    {
        if (pos.id == static_cast<u32>(-1))
            return end();

        if (m_allocator[pos.id].prev == static_cast<u32>(-1)) {
            pop_front();
            return begin();
        }

        if (m_allocator[pos.id].next == static_cast<u32>(-1)) {
            pop_back();
            return end();
        }

        auto prev = m_allocator[pos.id].prev;
        auto next = m_allocator[pos.id].next;

        m_allocator[prev].next = next;
        m_allocator[next].prev = prev;

        if constexpr (!std::is_trivial_v<T>)
            m_allocator[pos.id].value.~T();

        m_allocator.free(pos.id);

        return iterator(*this, next);
    }

    template<typename... Args>
    iterator emplace_front(Args&&... args) noexcept
    {
        irt_assert(m_allocator.can_alloc());

        u32 new_node = m_allocator.alloc_index();
        new (&m_allocator[new_node].value) T{ std::forward<Args>(args)... };

        u32 first, last;
        unpack_doubleword(m_list, &first, &last);

        if (m_list == static_cast<u64>(-1)) {
            m_allocator[new_node].prev = static_cast<u32>(-1);
            m_allocator[new_node].next = static_cast<u32>(-1);
            first                      = new_node;
            last                       = new_node;
        } else {
            m_allocator[new_node].prev = static_cast<u32>(-1);
            m_allocator[new_node].next = first;
            m_allocator[first].prev    = new_node;
            first                      = new_node;
        }

        m_list = make_doubleword(first, last);

        return iterator(*this, new_node);
    }

    template<typename... Args>
    iterator emplace_back(Args&&... args) noexcept
    {
        irt_assert(m_allocator.can_alloc());

        u32 new_node = m_allocator.alloc_index();
        new (&m_allocator[new_node].value) T{ std::forward<Args>(args)... };

        u32 first, last;
        unpack_doubleword(m_list, &first, &last);

        if (m_list == static_cast<u64>(-1)) {
            m_allocator[new_node].prev = static_cast<u32>(-1);
            m_allocator[new_node].next = static_cast<u32>(-1);
            first                      = new_node;
            last                       = new_node;
        } else {
            m_allocator[new_node].next = static_cast<u32>(-1);
            m_allocator[new_node].prev = last;
            m_allocator[last].next     = new_node;
            last                       = new_node;
        }

        m_list = make_doubleword(first, last);

        return iterator(*this, new_node);
    }

    iterator push_back(const T& t) noexcept
    {
        irt_assert(m_allocator.can_alloc());

        u32 new_node = m_allocator.alloc_index();
        new (&m_allocator[new_node].value) T(t);

        u32 first, last;
        unpack_doubleword(m_list, &first, &last);

        if (m_list == static_cast<u64>(-1)) {
            m_allocator[new_node].prev = static_cast<u32>(-1);
            m_allocator[new_node].next = static_cast<u32>(-1);
            first                      = new_node;
            last                       = new_node;
        } else {
            m_allocator[new_node].next = static_cast<u32>(-1);
            m_allocator[new_node].prev = last;
            m_allocator[last].next     = new_node;
            last                       = new_node;
        }

        m_list = make_doubleword(first, last);

        return iterator(*this, new_node);
    }

    void pop_front() noexcept
    {
        if (m_list == static_cast<u64>(-1))
            return;

        u32 begin, end;
        unpack_doubleword(m_list, &begin, &end);

        u32 to_delete = begin;
        begin         = m_allocator[to_delete].next;

        if (begin == static_cast<u32>(-1))
            end = static_cast<u32>(-1);
        else
            m_allocator[begin].prev = static_cast<u32>(-1);

        if constexpr (!std::is_trivial_v<T>)
            m_allocator[to_delete].value.~T();

        m_allocator.free(to_delete);
        m_list = make_doubleword(begin, end);
    }

    void pop_back() noexcept
    {
        if (m_list == static_cast<u64>(-1))
            return;

        u32 begin, end;
        unpack_doubleword(m_list, &begin, &end);

        u32 to_delete = end;
        end           = m_allocator[to_delete].prev;

        if (end == static_cast<u32>(-1))
            begin = static_cast<u32>(-1);
        else
            m_allocator[end].next = static_cast<u32>(-1);

        if constexpr (!std::is_trivial_v<T>)
            m_allocator[to_delete].value.~T();

        m_allocator.free(to_delete);
        m_list = make_doubleword(begin, end);
    }
};

template<typename T>
class list_view_const
{
public:
    using node_type       = list_view_node<T>;
    using allocator_type  = block_allocator<node_type>;
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using this_type       = list_view_const<T>;
    using iterator        = list_view_iterator_impl<this_type, T>;
    using const_iterator  = list_view_iterator_impl<this_type, const T>;

    template<typename X, typename Y>
    friend class list_view_iterator_impl;

private:
    const allocator_type& m_allocator;
    const u64             m_list;

public:
    list_view_const(const allocator_type& allocator, const u64 id) noexcept
      : m_allocator(allocator)
      , m_list(id)
    {}

    ~list_view_const() noexcept = default;

    bool empty() const noexcept { return m_list == static_cast<u64>(-1); }

    const_iterator begin() noexcept
    {
        return const_iterator(*this, unpack_doubleword_left(m_list));
    }

    const_iterator end() noexcept
    {
        return const_iterator(*this, static_cast<u32>(-1));
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(*this, unpack_doubleword_left(m_list));
    }

    const_iterator end() const noexcept
    {
        return const_iterator(*this, static_cast<u32>(-1));
    }

    const_reference front() noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_left(m_list)].value;
    }

    const_reference front() const noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_left(m_list)].value;
    }

    const_reference back() noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_right(m_list)].value;
    }

    const_reference back() const noexcept
    {
        irt_assert(!empty());

        return m_allocator[unpack_doubleword_right(m_list)].value;
    }
};

/*****************************************************************************
 *
 * data-array
 *
 ****************************************************************************/

enum class model_id : u64;
enum class dynamics_id : u64;
enum class message_id : u64;
enum class observer_id : u64;

template<typename T>
constexpr u32 get_index(T identifier) noexcept
{
    static_assert(std::is_enum<T>::value, "Identifier must be a enumeration");

    return unpack_doubleword_right(
      static_cast<std::underlying_type_t<T>>(identifier));
}

template<typename T>
constexpr u32 get_key(T identifier) noexcept
{
    static_assert(std::is_enum<T>::value, "Identifier must be a enumeration");

    return unpack_doubleword_left(
      static_cast<std::underlying_type_t<T>>(identifier));
}

template<typename T>
constexpr u32 get_max_size() noexcept
{
    return std::numeric_limits<u32>::max();
}

template<typename T>
constexpr bool is_valid(T identifier) noexcept
{
    return get_key(identifier) > 0;
}

template<typename T>
constexpr T make_id(u32 key, u32 index) noexcept
{
    return static_cast<T>(make_doubleword(key, index));
}

template<typename T>
constexpr u32 make_next_key(u32 key) noexcept
{
    return key == static_cast<u32>(-1) ? 1u : key + 1;
}

//! @brief An optimized fixed size array for dynamics objects.
//!
//! A container to handle everything from trivial, pod or object.
//! - linear memory/iteration
//! - O(1) alloc/free
//! - stable indices
//! - weak references
//! - zero overhead dereferences
//!
//! @tparam T The type of object the data_array holds.
//! @tparam Identifier A enum class identifier to store identifier unsigned
//!     number.
//! @todo Make Identifier split key|id automatically for all unsigned.
template<typename T, typename Identifier>
class data_array
{
private:
    static_assert(
      std::is_enum<Identifier>::value,
      "Identifier must be a enumeration: enum class id : unsigned {};");

    struct item
    {
        T          item;
        Identifier id;
    };

    item* m_items     = nullptr; // items array.
    u32   m_max_size  = 0;       // number of valid item in array.
    u32   m_max_used  = 0;       // highest index ever allocated in array.
    u32   m_capacity  = 0;       // capacity of the array.
    u32   m_next_key  = 1;       // [1..2^32] (don't let == 0)
    u32   m_free_head = none;    // index of first free entry

public:
    using identifier_type = Identifier;
    using value_type      = T;

    static constexpr u32 none = std::numeric_limits<u32>::max();

    data_array() = default;

    ~data_array() noexcept
    {
        clear();

        if (m_items)
            g_free_fn(m_items);
    }

    //! @brief Initialize the underlying byffer of T.
    //!
    //! @return @c is_success(status) or is_bad(status).
    status init(std::size_t capacity) noexcept
    {
        clear();

        if (capacity > get_max_size<Identifier>())
            return status::data_array_init_capacity_error;

        m_items = static_cast<item*>(g_alloc_fn(capacity * sizeof(item)));
        if (!m_items)
            return status::data_array_not_enough_memory;

        m_max_size  = 0;
        m_max_used  = 0;
        m_capacity  = static_cast<u32>(capacity);
        m_next_key  = 1;
        m_free_head = none;

        return status::success;
    }

    //! @brief Resets data members
    //!
    //! Run destructor on outstanding items and re-initialize of size.
    void clear() noexcept
    {
        if constexpr (!std::is_trivial_v<T>) {
            for (u32 i = 0; i != m_max_used; ++i) {
                if (is_valid(m_items[i].id)) {
                    m_items[i].item.~T();
                    m_items[i].id = static_cast<identifier_type>(0);
                }
            }
        }

        m_max_size  = 0;
        m_max_used  = 0;
        m_next_key  = 1;
        m_free_head = none;
    }

    //! @brief Alloc a new element.
    //!
    //! If m_max_size == m_capacity then this function will abort. Before using
    //! this function, tries @c !can_alloc() for example otherwise use the @c
    //! try_alloc function.
    //!
    //! Use @c m_free_head if not empty or use a new items from buffer
    //! (@m_item[max_used++]). The id is set to from @c next_key++ << 32) |
    //! index to build unique identifier.
    //!
    //! @return A reference to the newly allocated element.
    template<typename... Args>
    T& alloc(Args&&... args) noexcept
    {
        assert(can_alloc(1) && "check alloc() with full() before using use.");

        u32 new_index;

        if (m_free_head != none) {
            new_index = m_free_head;
            if (is_valid(m_items[m_free_head].id))
                m_free_head = none;
            else
                m_free_head = get_index(m_items[m_free_head].id);
        } else {
            new_index = m_max_used++;
        }

        new (&m_items[new_index].item) T(std::forward<Args>(args)...);

        m_items[new_index].id = make_id<Identifier>(m_next_key, new_index);
        m_next_key            = make_next_key<Identifier>(m_next_key);

        ++m_max_size;

        return m_items[new_index].item;
    }

    //! @brief Alloc a new element.
    //!
    //! Use @c m_free_head if not empty or use a new items from buffer
    //! (@m_item[max_used++]). The id is set to from @c next_key++ << 32) |
    //! index to build unique identifier.
    //!
    //! @return A pair with a boolean if the allocation success and a pointer
    //! to the newly element.
    template<typename... Args>
    std::pair<bool, T*> try_alloc(Args&&... args) noexcept
    {
        if (!can_alloc(1))
            return { false, nullptr };

        u32 new_index;

        if (m_free_head != none) {
            new_index = m_free_head;
            if (is_valid(m_items[m_free_head].id))
                m_free_head = none;
            else
                m_free_head = get_index(m_items[m_free_head].id);
        } else {
            new_index = m_max_used++;
        }

        new (&m_items[new_index].item) T(std::forward<Args>(args)...);

        m_items[new_index].id = make_id<Identifier>(m_next_key, new_index);
        m_next_key            = make_next_key<Identifier>(m_next_key);

        ++m_max_size;

        return { true, &m_items[new_index].item };
    }

    //! @brief Free the element @c t.
    //!
    //! Internally, puts the elelent @c t entry on free list and use id to
    //! store next.
    void free(T& t) noexcept
    {
        auto id    = get_id(t);
        auto index = get_index(id);

        irt_assert(&m_items[index] == static_cast<void*>(&t));
        irt_assert(m_items[index].id == id);
        irt_assert(is_valid(id));

        if constexpr (!std::is_trivial_v<T>)
            m_items[index].item.~T();

        m_items[index].id = static_cast<Identifier>(m_free_head);
        m_free_head       = index;

        --m_max_size;
    }

    //! @brief Free the element pointer by @c id.
    //!
    //! Internally, puts the element @c id entry on free list and use id to
    //! store next.
    void free(Identifier id) noexcept
    {
        auto index = get_index(id);

        irt_assert(m_items[index].id == id);
        irt_assert(is_valid(id));

        if constexpr (std::is_trivial_v<T>)
            m_items[index].item.~T();

        m_items[index].id = static_cast<Identifier>(m_free_head);
        m_free_head       = index;

        --m_max_size;
    }

    //! @brief Accessor to the id part of the item
    //!
    //! @return @c Identifier.
    Identifier get_id(const T* t) const noexcept
    {
        irt_assert(t != nullptr);

        auto* ptr = reinterpret_cast<const item*>(t);
        return ptr->id;
    }

    //! @brief Accessor to the id part of the item
    //!
    //! @return @c Identifier.
    Identifier get_id(const T& t) const noexcept
    {
        auto* ptr = reinterpret_cast<const item*>(&t);
        return ptr->id;
    }

    //! @brief Accessor to the item part of the id.
    //!
    //! @return @c T
    T& get(Identifier id) noexcept { return m_items[get_index(id)].item; }

    //! @brief Accessor to the item part of the id.
    //!
    //! @return @c T
    const T& get(Identifier id) const noexcept
    {
        return m_items[get_index(id)].item;
    }

    //! @brief Get a T from an ID.
    //!
    //! @details Validates ID, then returns item, returns null if invalid.
    //! For cases like AI references and others where 'the thing might have
    //! been deleted out from under me.
    //!
    //! @param id Identifier to get.
    //! @return T or nullptr
    T* try_to_get(Identifier id) const noexcept
    {
        if (get_key(id)) {
            auto index = get_index(id);
            if (m_items[index].id == id)
                return &m_items[index].item;
        }

        return nullptr;
    }

    //! @brief Get a T directly from the index in the array.
    //!
    //! @param index The array.
    //! @return T or nullptr.
    T* try_to_get(u32 index) const noexcept
    {
        irt_assert(index < m_max_used);

        if (is_valid(m_items[index].id))
            return &m_items[index].item;

        return nullptr;
    }

    //! @brief Return next valid item.
    //! @code
    //! data_array<int> d;
    //! ...
    //! int* value = nullptr;
    //! while (d.next(value)) {
    //!     std::cout << *value << ' ';
    //! }
    //! @endcode
    //!
    //! Loop item where @c id & 0xffffffff00000000 != 0 (i.e. items not on
    //! free list).
    //!
    //! @return true if the paramter @c t is valid false otherwise.
    bool next(T*& t) const noexcept
    {
        u32 index;

        if (t) {
            auto id = get_id(*t);
            index   = get_index(id);
            ++index;

            for (; index < m_max_used; ++index) {
                if (is_valid(m_items[index].id)) {
                    t = &m_items[index].item;
                    return true;
                }
            }
        } else {
            for (index = 0; index < m_max_used; ++index) {
                if (is_valid(m_items[index].id)) {
                    t = &m_items[index].item;
                    return true;
                }
            }
        }

        return false;
    }

    constexpr bool full() const noexcept
    {
        return m_free_head == none && m_max_used == m_capacity;
    }

    constexpr sz size() const noexcept { return m_max_size; }

    constexpr bool can_alloc(const sz nb) const noexcept
    {
        const u64 capacity = m_capacity;
        const u64 max_size = m_max_size;

        return capacity - max_size >= nb;
    }

    constexpr bool can_alloc() const noexcept
    {
        return m_capacity - m_max_size >= 1u;
    }

    constexpr u32 max_size() const noexcept { return m_max_size; }

    constexpr u32 max_used() const noexcept { return m_max_used; }

    constexpr u32 capacity() const noexcept { return m_capacity; }

    constexpr u32 next_key() const noexcept { return m_next_key; }

    constexpr bool is_free_list_empty() const noexcept
    {
        return m_free_head == none;
    }
};

struct record
{
    record() noexcept = default;

    record(real x_dot_, time date_) noexcept
      : x_dot{ x_dot_ }
      , date{ date_ }
    {}

    real x_dot{ 0 };
    time date{ time_domain<time>::infinity };
};

//! @brief Pairing heap implementation.
//!
//! A pairing heap is a type of heap data structure with relatively simple
//! implementation and excellent practical amortized performance, introduced
//! by Michael Fredman, Robert Sedgewick, Daniel Sleator, and Robert Tarjan
//! in 1986.
//!
//! https://en.wikipedia.org/wiki/Pairing_heap
class heap
{
private:
    struct node
    {
        time     tn;
        model_id id;

        node* prev;
        node* next;
        node* child;
    };

    size_t m_size{ 0 };
    size_t max_size{ 0 };
    size_t capacity{ 0 };
    node*  root{ nullptr };
    node*  nodes{ nullptr };
    node*  free_list{ nullptr };

public:
    using handle = node*;

    heap() = default;

    ~heap() noexcept
    {
        if (nodes)
            g_free_fn(nodes);
    }

    status init(size_t new_capacity) noexcept
    {
        if (new_capacity == 0)
            return status::head_allocator_bad_capacity;

        if (new_capacity != capacity) {
            if (nodes)
                g_free_fn(nodes);

            nodes = static_cast<node*>(g_alloc_fn(new_capacity * sizeof(node)));
            if (nodes == nullptr)
                return status::head_allocator_not_enough_memory;
        }

        m_size    = 0;
        max_size  = 0;
        capacity  = new_capacity;
        root      = nullptr;
        free_list = nullptr;

        return status::success;
    }

    void clear()
    {
        m_size    = 0;
        max_size  = 0;
        root      = nullptr;
        free_list = nullptr;
    }

    handle insert(time tn, model_id id) noexcept
    {
        node* new_node;

        if (free_list) {
            new_node  = free_list;
            free_list = free_list->next;
        } else {
            new_node = &nodes[max_size++];
        }

        new_node->tn    = tn;
        new_node->id    = id;
        new_node->prev  = nullptr;
        new_node->next  = nullptr;
        new_node->child = nullptr;

        insert(new_node);

        return new_node;
    }

    void destroy(handle elem) noexcept
    {
        irt_assert(elem);

        if (m_size == 0) {
            clear();
        } else {
            elem->prev  = nullptr;
            elem->child = nullptr;
            elem->id    = static_cast<model_id>(0);

            elem->next = free_list;
            free_list  = elem;
        }
    }

    void insert(handle elem) noexcept
    {
        elem->prev  = nullptr;
        elem->next  = nullptr;
        elem->child = nullptr;

        ++m_size;

        if (root == nullptr)
            root = elem;
        else
            root = merge(elem, root);
    }

    void remove(handle elem) noexcept
    {
        irt_assert(elem);

        if (elem == root) {
            pop();
            return;
        }

        irt_assert(m_size > 0);

        m_size--;
        detach_subheap(elem);

        if (elem->prev) { /* Not use pop() before. Use in interactive code */
            elem = merge_subheaps(elem);
            root = merge(root, elem);
        }
    }

    handle pop() noexcept
    {
        irt_assert(m_size > 0);

        m_size--;

        auto* top = root;

        if (top->child == nullptr)
            root = nullptr;
        else
            root = merge_subheaps(top);

        top->child = top->next = top->prev = nullptr;

        return top;
    }

    void decrease(handle elem) noexcept
    {
        if (elem->prev == nullptr)
            return;

        detach_subheap(elem);
        root = merge(root, elem);
    }

    void increase(handle elem) noexcept
    {
        remove(elem);
        insert(elem);
    }

    size_t size() const noexcept { return m_size; }

    size_t full() const noexcept { return m_size == capacity; }

    bool empty() const noexcept { return root == nullptr; }

    handle top() const noexcept { return root; }

    void merge(heap& src) noexcept
    {
        if (this == &src)
            return;

        if (root == nullptr) {
            root = src.root;
            return;
        }

        root = merge(root, src.root);
        m_size += src.m_size;
    }

private:
    static node* merge(node* a, node* b) noexcept
    {
        if (a->tn < b->tn) {
            if (a->child != nullptr)
                a->child->prev = b;

            if (b->next != nullptr)
                b->next->prev = a;

            a->next  = b->next;
            b->next  = a->child;
            a->child = b;
            b->prev  = a;

            return a;
        }

        if (b->child != nullptr)
            b->child->prev = a;

        if (a->prev != nullptr && a->prev->child != a)
            a->prev->next = b;

        b->prev  = a->prev;
        a->prev  = b;
        a->next  = b->child;
        b->child = a;

        return b;
    }

    static node* merge_right(node* a) noexcept
    {
        node* b{ nullptr };

        for (; a != nullptr; a = b->next) {
            if ((b = a->next) == nullptr)
                return a;

            b = merge(a, b);
        }

        return b;
    }

    static node* merge_left(node* a) noexcept
    {
        node* b = a->prev;
        for (; b != nullptr; b = a->prev)
            a = merge(b, a);

        return a;
    }

    static node* merge_subheaps(node* a) noexcept
    {
        a->child->prev = nullptr;

        node* e = merge_right(a->child);
        return merge_left(e);
    }

    void detach_subheap(node* elem) noexcept
    {
        if (elem->prev->child == elem)
            elem->prev->child = elem->next;
        else
            elem->prev->next = elem->next;

        if (elem->next != nullptr)
            elem->next->prev = elem->prev;

        elem->prev = nullptr;
        elem->next = nullptr;
    }
};

struct simulation;

/*****************************************************************************
 *
 * @c source and @c source_id are data from files or random generators.
 *
 ****************************************************************************/

struct source
{
    enum class operation_type
    {
        initialize, // Use to initialize the buffer at simulation init step.
        update,     // Use to update the buffer when all values are read.
        finalize    // Use to clear the buffer at simulation finalize step.
    };

    double* buffer = nullptr;
    u64     id   = 0; // The identifier of the external source (see operation())
    i32     type = -1; // The type of the external source (see operation())
    i32     size = 0;
    i32     index = 0;

    void reset() noexcept
    {
        buffer = nullptr;
        size   = 0;
        index  = 0;
        type   = -1;
        id     = 0;
    }

    void clear() noexcept
    {
        buffer = nullptr;
        size   = 0;
        index  = 0;
    }

    bool next(double& value) noexcept
    {
        if (index >= size)
            return false;

        value = buffer[index++];

        return true;
    }
};

/**
 * @brief Call in the initialize function of the models.
 * @param sim The simulation.
 * @param src The sources.
 * @return
 */
inline status initialize_source(simulation& sim, source& src) noexcept;

inline status update_source(simulation& sim, source& src, double& val) noexcept;

inline status finalize_source(simulation& sim, source& src) noexcept;

/*****************************************************************************
 *
 * DEVS Model / Simulation entities
 *
 ****************************************************************************/

enum class dynamics_type : i32
{
    qss1_integrator,
    qss1_multiplier,
    qss1_cross,
    qss1_power,
    qss1_square,
    qss1_sum_2,
    qss1_sum_3,
    qss1_sum_4,
    qss1_wsum_2,
    qss1_wsum_3,
    qss1_wsum_4,

    qss2_integrator,
    qss2_multiplier,
    qss2_cross,
    qss2_power,
    qss2_square,
    qss2_sum_2,
    qss2_sum_3,
    qss2_sum_4,
    qss2_wsum_2,
    qss2_wsum_3,
    qss2_wsum_4,

    qss3_integrator,
    qss3_multiplier,
    qss3_cross,
    qss3_power,
    qss3_square,
    qss3_sum_2,
    qss3_sum_3,
    qss3_sum_4,
    qss3_wsum_2,
    qss3_wsum_3,
    qss3_wsum_4,

    integrator,
    quantifier,
    adder_2,
    adder_3,
    adder_4,
    mult_2,
    mult_3,
    mult_4,

    counter,

    queue,
    dynamic_queue,
    priority_queue,

    generator,
    constant,
    cross,
    time_func,
    accumulator_2,
    filter,
    flow
};

constexpr i8 dynamics_type_last() noexcept
{
    return static_cast<i8>(dynamics_type::flow);
}

constexpr sz dynamics_type_size() noexcept
{
    return static_cast<sz>(dynamics_type_last() + 1);
}

struct observer;

struct observer
{
    enum class status
    {
        initialize,
        run,
        finalize
    };

    using update_fn = void (*)(const observer&,
                               const dynamics_type,
                               const time,
                               const time,
                               const observer::status);

    observer(const char* name_, update_fn cb_, void* user_data_) noexcept
      : cb(cb_)
      , name(name_)
      , user_data(user_data_)
    {}

    update_fn           cb;
    small_string<8>     name;
    model_id            model = static_cast<model_id>(0);
    observation_message msg;
    void*               user_data = nullptr;
};

struct node
{
    node() = default;

    node(const model_id model_, const i8 port_index_) noexcept
      : model(model_)
      , port_index(port_index_)
    {}

    model_id model      = undefined<model_id>();
    i8       port_index = 0;
};

struct output_message
{
    message  msg;
    model_id model;
    i8       port;
};

using output_port = u64;
using input_port  = u64;

inline bool have_message(const u64 port) noexcept
{
    return port != static_cast<u64>(-1);
}

bool can_alloc_message(const simulation& sim, int alloc_number) noexcept;
bool can_alloc_node(const simulation& sim, int alloc_number) noexcept;
bool can_alloc_dated_message(const simulation& sim, int alloc_number) noexcept;

list_view<message> append_message(simulation& sim, input_port& port) noexcept;
list_view_const<message> get_message(const simulation& sim,
                                     const input_port  port) noexcept;

list_view<node>       append_node(simulation& sim, output_port& port) noexcept;
list_view_const<node> get_node(const simulation& sim,
                               const output_port port) noexcept;

list_view<record>       append_archive(simulation& sim, u64& id) noexcept;
list_view_const<record> get_archive(const simulation& sim,
                                    const u64         id) noexcept;

list_view<dated_message>       append_dated_message(simulation& sim,
                                                    u64&        id) noexcept;
list_view_const<dated_message> get_dated_message(const simulation& sim,
                                                 const u64         id) noexcept;

status send_message(simulation&  sim,
                    output_port& p,
                    real         r1,
                    real         r2 = zero,
                    real         r3 = zero) noexcept;

/////////////////////////////////////////////////////////////////////////////
//
// Some template type-trait to detect function and attribute in DEVS model.
//
/////////////////////////////////////////////////////////////////////////////

namespace detail {

template<typename, template<typename...> class, typename...>
struct is_detected : std::false_type
{};

template<template<class...> class Operation, typename... Arguments>
struct is_detected<std::void_t<Operation<Arguments...>>,
                   Operation,
                   Arguments...> : std::true_type
{};

template<typename T, T>
struct helper
{};

} // namespace detail

template<template<class...> class Operation, typename... Arguments>
using is_detected = detail::is_detected<std::void_t<>, Operation, Arguments...>;

template<template<class...> class Operation, typename... Arguments>
constexpr bool is_detected_v =
  detail::is_detected<std::void_t<>, Operation, Arguments...>::value;

template<class T>
using lambda_function_t =
  decltype(detail::helper<status (T::*)(simulation&), &T::lambda>{});

template<class T>
using transition_function_t =
  decltype(detail::helper<status (T::*)(simulation&, time, time, time),
                          &T::transition>{});

template<class T>
using observation_function_t =
  decltype(detail::helper<observation_message (T::*)(const time) const,
                          &T::observation>{});

template<class T>
using initialize_function_t =
  decltype(detail::helper<status (T::*)(simulation&), &T::initialize>{});

template<class T>
using finalize_function_t =
  decltype(detail::helper<status (T::*)(simulation&), &T::finalize>{});

template<typename T>
using has_input_port_t = decltype(&T::x);

template<typename T>
using has_output_port_t = decltype(&T::y);

struct integrator
{
    input_port  x[3];
    output_port y[1];
    time        sigma = time_domain<time>::zero;

    enum port_name
    {
        port_quanta,
        port_x_dot,
        port_reset
    };

    enum class state
    {
        init,
        wait_for_quanta,
        wait_for_x_dot,
        wait_for_both,
        running
    };

    real default_current_value = 0.0;
    real default_reset_value   = 0.0;
    u64  archive               = static_cast<u64>(-1);

    real  current_value     = 0.0;
    real  reset_value       = 0.0;
    real  up_threshold      = 0.0;
    real  down_threshold    = 0.0;
    real  last_output_value = 0.0;
    real  expected_value    = 0.0;
    bool  reset             = false;
    state st                = state::init;

    integrator() = default;

    integrator(const integrator& other) noexcept
      : default_current_value(other.default_current_value)
      , default_reset_value(other.default_reset_value)
      , archive(static_cast<u64>(-1))
      , current_value(other.current_value)
      , reset_value(other.reset_value)
      , up_threshold(other.up_threshold)
      , down_threshold(other.down_threshold)
      , last_output_value(other.last_output_value)
      , expected_value(other.expected_value)
      , reset(other.reset)
      , st(other.st)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        current_value     = default_current_value;
        reset_value       = default_reset_value;
        up_threshold      = 0.0;
        down_threshold    = 0.0;
        last_output_value = 0.0;
        expected_value    = 0.0;
        reset             = false;
        st                = state::init;
        archive           = static_cast<u64>(-1);
        sigma             = time_domain<time>::zero;

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        append_archive(sim, archive).clear();

        return status::success;
    }

    status external(simulation& sim, time t) noexcept
    {
        if (have_message(x[port_quanta])) {
            auto lst = get_message(sim, x[port_quanta]);
            for (const auto& msg : lst) {
                up_threshold   = msg.data[0];
                down_threshold = msg.data[1];

                if (st == state::wait_for_quanta)
                    st = state::running;

                if (st == state::wait_for_both)
                    st = state::wait_for_x_dot;
            }
        }

        if (have_message(x[port_x_dot])) {
            auto lst          = get_message(sim, x[port_x_dot]);
            auto archive_list = append_archive(sim, archive);
            for (const auto& msg : lst) {
                archive_list.emplace_back(msg.data[0], t);

                if (st == state::wait_for_x_dot)
                    st = state::running;

                if (st == state::wait_for_both)
                    st = state::wait_for_quanta;
            }
        }

        if (have_message(x[port_reset])) {
            auto lst = get_message(sim, x[port_reset]);
            for (const auto& msg : lst) {
                reset_value = msg.data[0];
                reset       = true;
            }
        }

        if (st == state::running) {
            current_value  = compute_current_value(sim, t);
            expected_value = compute_expected_value(sim);
        }

        return status::success;
    }

    status internal(simulation& sim, time t) noexcept
    {
        switch (st) {
        case state::running: {
            last_output_value = expected_value;
            auto lst          = append_archive(sim, archive);

            const real last_derivative_value = lst.back().x_dot;
            lst.clear();
            lst.emplace_back(last_derivative_value, t);
            current_value = expected_value;
            st            = state::wait_for_quanta;
            return status::success;
        }
        case state::init:
            st                = state::wait_for_both;
            last_output_value = current_value;
            return status::success;
        default:
            return status::model_integrator_internal_error;
        }
    }

    status transition(simulation& sim, time t, time /*e*/, time r) noexcept
    {
        if (!have_message(x[port_quanta]) && !have_message(x[port_x_dot]) &&
            !have_message(x[port_reset])) {
            irt_return_if_bad(internal(sim, t));
        } else {
            if (time_domain<time>::is_zero(r))
                irt_return_if_bad(internal(sim, t));

            irt_return_if_bad(external(sim, t));
        }

        return ta(sim);
    }

    status lambda(simulation& sim) noexcept
    {
        switch (st) {
        case state::running:
            return send_message(sim, y[0], expected_value);
        case state::init:
            return send_message(sim, y[0], current_value);
        default:
            return status::model_integrator_output_error;
        }

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { last_output_value };
    }

    status ta(simulation& sim) noexcept
    {
        if (st == state::running) {
            irt_return_if_fail(static_cast<u64>(-1) != archive,
                               status::model_integrator_running_without_x_dot);

            auto       lst                = get_archive(sim, archive);
            const auto current_derivative = lst.back().x_dot;

            if (current_derivative == time_domain<time>::zero) {
                sigma = time_domain<time>::infinity;
                return status::success;
            }

            if (current_derivative > 0) {
                irt_return_if_fail((up_threshold - current_value) >= 0,
                                   status::model_integrator_ta_with_bad_x_dot);

                sigma = (up_threshold - current_value) / current_derivative;
                return status::success;
            }

            irt_return_if_fail((down_threshold - current_value) <= 0,
                               status::model_integrator_ta_with_bad_x_dot);

            sigma = (down_threshold - current_value) / current_derivative;
            return status::success;
        }

        sigma = time_domain<time>::infinity;
        return status::success;
    }

    real compute_current_value(simulation& sim, time t) noexcept
    {
        if (static_cast<u64>(-1) == archive)
            return reset ? reset_value : last_output_value;

        auto lst  = get_archive(sim, archive);
        auto val  = reset ? reset_value : last_output_value;
        auto end  = lst.end();
        auto it   = lst.begin();
        auto next = lst.begin();

        if (next != end)
            ++next;

        for (; next != end; it = next++)
            val += (next->date - it->date) * it->x_dot;

        val += (t - lst.back().date) * lst.back().x_dot;

        if (up_threshold < val) {
            return up_threshold;
        } else if (down_threshold > val) {
            return down_threshold;
        } else {
            return val;
        }
    }

    real compute_expected_value(simulation& sim) noexcept
    {
        auto       lst                = get_archive(sim, archive);
        const auto current_derivative = lst.back().x_dot;

        if (current_derivative == 0)
            return current_value;

        if (current_derivative > 0)
            return up_threshold;

        return down_threshold;
    }
};

/*****************************************************************************
 *
 * Qss1 part
 *
 ****************************************************************************/

template<int QssLevel>
struct abstract_integrator;

template<>
struct abstract_integrator<1>
{
    input_port  x[2];
    output_port y[1];
    real        default_X  = zero;
    real        default_dQ = irt::real(0.01);
    real        X;
    real        q;
    real        u;
    time        sigma = time_domain<time>::zero;

    enum port_name
    {
        port_x_dot,
        port_reset
    };

    abstract_integrator() = default;

    abstract_integrator(const abstract_integrator& other) noexcept
      : default_X(other.default_X)
      , default_dQ(other.default_dQ)
      , X(other.X)
      , q(other.q)
      , u(other.u)
      , sigma(other.sigma)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        irt_return_if_fail(std::isfinite(default_X),
                           status::model_integrator_X_error);

        irt_return_if_fail(std::isfinite(default_dQ) && default_dQ > zero,
                           status::model_integrator_X_error);

        X = default_X;
        q = std::floor(X / default_dQ) * default_dQ;
        u = zero;

        sigma = time_domain<time>::zero;

        return status::success;
    }

    status external(simulation& sim, const time e) noexcept
    {
        auto lst     = get_message(sim, x[port_x_dot]);
        auto value_x = lst.front()[0];

        X += e * u;
        u = value_x;

        if (sigma != zero) {
            if (u == zero)
                sigma = time_domain<time>::infinity;
            else if (u > zero)
                sigma = (q + default_dQ - X) / u;
            else
                sigma = (q - default_dQ - X) / u;
        }

        return status::success;
    }

    status reset(simulation& sim) noexcept
    {
        auto  lst   = get_message(sim, x[port_reset]);
        auto& value = lst.front();

        X     = value[0];
        q     = X;
        sigma = time_domain<time>::zero;
        return status::success;
    }

    status internal() noexcept
    {
        X += sigma * u;
        q = X;

        sigma =
          u == zero ? time_domain<time>::infinity : default_dQ / std::abs(u);

        return status::success;
    }

    status transition(simulation& sim, time /*t*/, time e, time /*r*/) noexcept
    {
        if (!have_message(x[port_x_dot]) && !have_message(x[port_reset])) {
            irt_return_if_bad(internal());
        } else {
            if (have_message(x[port_reset])) {
                irt_return_if_bad(reset(sim));
            } else {
                irt_return_if_bad(external(sim, e));
            }
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], X + u * sigma);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { X, u };
    }
};

/*****************************************************************************
 *
 * Qss2 part
 *
 ****************************************************************************/

template<>
struct abstract_integrator<2>
{
    input_port  x[2];
    output_port y[1];
    real        default_X  = zero;
    real        default_dQ = irt::real(0.01);
    real        X;
    real        u;
    real        mu;
    real        q;
    real        mq;
    time        sigma = time_domain<time>::zero;

    enum port_name
    {
        port_x_dot,
        port_reset
    };

    abstract_integrator() = default;

    abstract_integrator(const abstract_integrator& other) noexcept
      : default_X(other.default_X)
      , default_dQ(other.default_dQ)
      , X(other.X)
      , u(other.u)
      , mu(other.mu)
      , q(other.q)
      , mq(other.mq)
      , sigma(other.sigma)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        irt_return_if_fail(std::isfinite(default_X),
                           status::model_integrator_X_error);

        irt_return_if_fail(std::isfinite(default_dQ) && default_dQ > zero,
                           status::model_integrator_X_error);

        X = default_X;

        u  = zero;
        mu = zero;
        q  = X;
        mq = zero;

        sigma = time_domain<time>::zero;

        return status::success;
    }

    status external(simulation& sim, const time e) noexcept
    {
        auto       lst         = get_message(sim, x[port_x_dot]);
        const real value_x     = lst.front()[0];
        const real value_slope = lst.front()[1];

        X += (u * e) + (mu / two) * (e * e);
        u  = value_x;
        mu = value_slope;

        if (sigma != zero) {
            q += mq * e;
            const real a = mu / two;
            const real b = u - mq;
            real       c = X - q + default_dQ;
            real       s;
            sigma = time_domain<time>::infinity;

            if (a == zero) {
                if (b != zero) {
                    s = -c / b;
                    if (s > zero)
                        sigma = s;

                    c = X - q - default_dQ;
                    s = -c / b;
                    if ((s > zero) && (s < sigma))
                        sigma = s;
                }
            } else {
                s = (-b + std::sqrt(b * b - four * a * c)) / two / a;
                if (s > zero)
                    sigma = s;

                s = (-b - std::sqrt(b * b - four * a * c)) / two / a;
                if ((s > zero) && (s < sigma))
                    sigma = s;

                c = X - q - default_dQ;
                s = (-b + std::sqrt(b * b - four * a * c)) / two / a;
                if ((s > zero) && (s < sigma))
                    sigma = s;

                s = (-b - std::sqrt(b * b - four * a * c)) / two / a;
                if ((s > zero) && (s < sigma))
                    sigma = s;
            }

            if (((X - q) > default_dQ) || ((q - X) > default_dQ))
                sigma = time_domain<time>::zero;
        }

        return status::success;
    }

    status internal() noexcept
    {
        X += u * sigma + mu / two * sigma * sigma;
        q = X;
        u += mu * sigma;
        mq = u;

        sigma = mu == zero ? time_domain<time>::infinity
                           : std::sqrt(two * default_dQ / std::abs(mu));

        return status::success;
    }

    status reset(simulation& sim) noexcept
    {
        auto lst         = get_message(sim, x[port_reset]);
        auto value_reset = lst.front()[0];

        X     = value_reset;
        q     = X;
        sigma = time_domain<time>::zero;
        return status::success;
    }

    status transition(simulation& sim, time /*t*/, time e, time /*r*/) noexcept
    {
        if (!have_message(x[port_x_dot]) && !have_message(x[port_reset]))
            irt_return_if_bad(internal());
        else {
            if (have_message(x[port_reset])) {
                irt_return_if_bad(reset(sim));
            } else {
                irt_return_if_bad(external(sim, e));
            }
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(
          sim, y[0], X + u * sigma + mu * sigma * sigma / two, u + mu * sigma);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { X, u, mu };
    }
};

template<>
struct abstract_integrator<3>
{
    input_port  x[2];
    output_port y[1];
    real        default_X  = zero;
    real        default_dQ = irt::real(0.01);
    real        X;
    real        u;
    real        mu;
    real        pu;
    real        q;
    real        mq;
    real        pq;
    time        sigma = time_domain<time>::zero;

    enum port_name
    {
        port_x_dot,
        port_reset
    };

    abstract_integrator() = default;

    abstract_integrator(const abstract_integrator& other) noexcept
      : default_X(other.default_dQ)
      , default_dQ(other.default_dQ)
      , X(other.X)
      , u(other.u)
      , mu(other.mu)
      , pu(other.pu)
      , q(other.q)
      , mq(other.mq)
      , pq(other.pq)
      , sigma(other.sigma)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        irt_return_if_fail(std::isfinite(default_X),
                           status::model_integrator_X_error);

        irt_return_if_fail(std::isfinite(default_dQ) && default_dQ > zero,
                           status::model_integrator_X_error);

        X     = default_X;
        u     = zero;
        mu    = zero;
        pu    = zero;
        q     = default_X;
        mq    = zero;
        pq    = zero;
        sigma = time_domain<time>::zero;

        return status::success;
    }

    status external(simulation& sim, const time e) noexcept
    {
        auto       lst              = get_message(sim, x[port_x_dot]);
        auto&      value            = lst.front();
        const real value_x          = value[0];
        const real value_slope      = value[1];
        const real value_derivative = value[2];

#if irt_have_numbers == 1
        constexpr real pi_div_3 = std::numbers::pi_v<real> / three;
#else
        constexpr real pi_div_3 = 1.0471975511965976;
#endif

        X  = X + u * e + (mu * e * e) / two + (pu * e * e * e) / three;
        u  = value_x;
        mu = value_slope;
        pu = value_derivative;

        if (sigma != zero) {
            q      = q + mq * e + pq * e * e;
            mq     = mq + two * pq * e;
            auto a = mu / two - pq;
            auto b = u - mq;
            auto c = X - q - default_dQ;
            auto s = zero;

            if (pu != zero) {
                a       = three * a / pu;
                b       = three * b / pu;
                c       = three * c / pu;
                auto v  = b - a * a / three;
                auto w  = c - b * a / three + two * a * a * a / real(27);
                auto i1 = -w / two;
                auto i2 = i1 * i1 + v * v * v / real(27);

                if (i2 > zero) {
                    i2     = std::sqrt(i2);
                    auto A = i1 + i2;
                    auto B = i1 - i2;

                    if (A > zero)
                        A = std::pow(A, one / three);
                    else
                        A = -std::pow(std::abs(A), one / three);
                    if (B > zero)
                        B = std::pow(B, one / three);
                    else
                        B = -std::pow(std::abs(B), one / three);

                    s = A + B - a / three;
                    if (s < zero)
                        s = time_domain<time>::infinity;
                } else if (i2 == zero) {
                    auto A = i1;
                    if (A > zero)
                        A = std::pow(A, one / three);
                    else
                        A = -std::pow(std::abs(A), one / three);
                    auto x1 = two * A - a / three;
                    auto x2 = -(A + a / three);
                    if (x1 < zero) {
                        if (x2 < zero) {
                            s = time_domain<time>::infinity;
                        } else {
                            s = x2;
                        }
                    } else if (x2 < zero) {
                        s = x1;
                    } else if (x1 < x2) {
                        s = x1;
                    } else {
                        s = x2;
                    }
                } else {
                    auto arg = w * std::sqrt(real(27) / (-v)) / (two * v);
                    arg      = std::acos(arg) / three;
                    auto y1  = two * std::sqrt(-v / three);
                    auto y2  = -y1 * std::cos(pi_div_3 - arg) - a / three;
                    auto y3  = -y1 * std::cos(pi_div_3 + arg) - a / three;
                    y1       = y1 * std::cos(arg) - a / three;
                    if (y1 < zero) {
                        s = time_domain<time>::infinity;
                    } else if (y3 < zero) {
                        s = y1;
                    } else if (y2 < zero) {
                        s = y3;
                    } else {
                        s = y2;
                    }
                }
                c  = c + real(6) * default_dQ / pu;
                w  = c - b * a / three + two * a * a * a / real(27);
                i1 = -w / two;
                i2 = i1 * i1 + v * v * v / real(27);
                if (i2 > zero) {
                    i2     = std::sqrt(i2);
                    auto A = i1 + i2;
                    auto B = i1 - i2;
                    if (A > zero)
                        A = std::pow(A, one / three);
                    else
                        A = -std::pow(std::abs(A), one / three);
                    if (B > zero)
                        B = std::pow(B, one / three);
                    else
                        B = -std::pow(std::abs(B), one / three);
                    sigma = A + B - a / three;

                    if (s < sigma || sigma < zero) {
                        sigma = s;
                    }
                } else if (i2 == zero) {
                    auto A = i1;
                    if (A > zero)
                        A = std::pow(A, one / three);
                    else
                        A = -std::pow(std::abs(A), one / three);
                    auto x1 = two * A - a / three;
                    auto x2 = -(A + a / three);
                    if (x1 < zero) {
                        if (x2 < zero) {
                            sigma = time_domain<time>::infinity;
                        } else {
                            sigma = x2;
                        }
                    } else if (x2 < zero) {
                        sigma = x1;
                    } else if (x1 < x2) {
                        sigma = x1;
                    } else {
                        sigma = x2;
                    }
                    if (s < sigma) {
                        sigma = s;
                    }
                } else {
                    auto arg = w * std::sqrt(real(27) / (-v)) / (two * v);
                    arg      = std::acos(arg) / three;
                    auto y1  = two * std::sqrt(-v / three);
                    auto y2  = -y1 * std::cos(pi_div_3 - arg) - a / three;
                    auto y3  = -y1 * std::cos(pi_div_3 + arg) - a / three;
                    y1       = y1 * std::cos(arg) - a / three;
                    if (y1 < zero) {
                        sigma = time_domain<time>::infinity;
                    } else if (y3 < zero) {
                        sigma = y1;
                    } else if (y2 < zero) {
                        sigma = y3;
                    } else {
                        sigma = y2;
                    }
                    if (s < sigma) {
                        sigma = s;
                    }
                }
            } else {
                if (a != zero) {
                    auto x1 = b * b - four * a * c;
                    if (x1 < zero) {
                        s = time_domain<time>::infinity;
                    } else {
                        x1      = std::sqrt(x1);
                        auto x2 = (-b - x1) / two / a;
                        x1      = (-b + x1) / two / a;
                        if (x1 < zero) {
                            if (x2 < zero) {
                                s = time_domain<time>::infinity;
                            } else {
                                s = x2;
                            }
                        } else if (x2 < zero) {
                            s = x1;
                        } else if (x1 < x2) {
                            s = x1;
                        } else {
                            s = x2;
                        }
                    }
                    c  = c + two * default_dQ;
                    x1 = b * b - four * a * c;
                    if (x1 < zero) {
                        sigma = time_domain<time>::infinity;
                    } else {
                        x1      = std::sqrt(x1);
                        auto x2 = (-b - x1) / two / a;
                        x1      = (-b + x1) / two / a;
                        if (x1 < zero) {
                            if (x2 < zero) {
                                sigma = time_domain<time>::infinity;
                            } else {
                                sigma = x2;
                            }
                        } else if (x2 < zero) {
                            sigma = x1;
                        } else if (x1 < x2) {
                            sigma = x1;
                        } else {
                            sigma = x2;
                        }
                    }
                    if (s < sigma)
                        sigma = s;
                } else {
                    if (b != zero) {
                        auto x1 = -c / b;
                        auto x2 = x1 - two * default_dQ / b;
                        if (x1 < zero)
                            x1 = time_domain<time>::infinity;
                        if (x2 < zero)
                            x2 = time_domain<time>::infinity;
                        if (x1 < x2) {
                            sigma = x1;
                        } else {
                            sigma = x2;
                        }
                    }
                }
            }

            if ((std::abs(X - q)) > default_dQ)
                sigma = time_domain<time>::zero;
        }

        return status::success;
    }

    status internal() noexcept
    {
        X = X + u * sigma + (mu * sigma * sigma) / two +
            (pu * sigma * sigma * sigma) / three;
        q  = X;
        u  = u + mu * sigma + pu * std::pow(sigma, two);
        mq = u;
        mu = mu + two * pu * sigma;
        pq = mu / two;

        sigma = pu == zero
                  ? time_domain<time>::infinity
                  : std::pow(std::abs(three * default_dQ / pu), one / three);

        return status::success;
    }

    status reset(simulation& sim) noexcept
    {
        auto span = get_message(sim, x[port_reset]);

        X     = span.front()[0];
        q     = X;
        sigma = time_domain<time>::zero;

        return status::success;
    }

    status transition(simulation& sim, time /*t*/, time e, time /*r*/) noexcept
    {
        if (!have_message(x[port_x_dot]) && !have_message(x[port_reset]))
            irt_return_if_bad(internal());
        else {
            if (have_message(x[port_reset]))
                irt_return_if_bad(reset(sim));
            else
                irt_return_if_bad(external(sim, e));
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim,
                            y[0],
                            X + u * sigma + (mu * sigma * sigma) / two +
                              (pu * sigma * sigma * sigma) / three,
                            u + mu * sigma + pu * sigma * sigma,
                            mu / two + pu * sigma);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { X, u, mu, pu };
    }
};

using qss1_integrator = abstract_integrator<1>;
using qss2_integrator = abstract_integrator<2>;
using qss3_integrator = abstract_integrator<3>;

template<int QssLevel>
struct abstract_power
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");

    input_port  x[1];
    output_port y[1];
    time        sigma;

    real value[QssLevel];
    real default_n;

    abstract_power() noexcept = default;

    abstract_power(const abstract_power& other) noexcept
      : default_n(other.default_n)
    {
        std::copy_n(other.value, QssLevel, value);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(value, QssLevel, zero);
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1)
            return send_message(sim, y[0], std::pow(value[0], default_n));

        if constexpr (QssLevel == 2)
            return send_message(sim,
                                y[0],
                                std::pow(value[0], default_n),
                                default_n * std::pow(value[0], default_n - 1) *
                                  value[1]);

        if constexpr (QssLevel == 3)
            return send_message(
              sim,
              y[0],
              std::pow(value[0], default_n),
              default_n * std::pow(value[0], default_n - 1) * value[1],
              default_n * (default_n - 1) * std::pow(value[0], default_n - 2) *
                  (value[1] * value[1] / two) +
                default_n * std::pow(value[0], default_n - 1) * value[2]);

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        sigma = time_domain<time>::infinity;

        if (have_message(x[0])) {
            auto  list = get_message(sim, x[0]);
            auto& msg  = list.front();

            if constexpr (QssLevel == 1) {
                value[0] = msg[0];
            }

            if constexpr (QssLevel == 2) {
                value[0] = msg[0];
                value[1] = msg[1];
            }

            if constexpr (QssLevel == 3) {
                value[0] = msg[0];
                value[1] = msg[1];
                value[2] = msg[2];
            }

            sigma = time_domain<time>::zero;
        }

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { value[0] };
    }
};

using qss1_power = abstract_power<1>;
using qss2_power = abstract_power<2>;
using qss3_power = abstract_power<3>;

template<int QssLevel>
struct abstract_square
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");

    input_port  x[1];
    output_port y[1];
    time        sigma;

    real value[QssLevel];

    abstract_square() noexcept = default;

    abstract_square(const abstract_square& other) noexcept
    {
        std::copy_n(other.value, QssLevel, value);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(value, QssLevel, zero);
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1)
            return send_message(sim, y[0], value[0] * value[0]);

        if constexpr (QssLevel == 2)
            return send_message(
              sim, y[0], value[0] * value[0], two * value[0] * value[1]);

        if constexpr (QssLevel == 3)
            return send_message(sim,
                                y[0],
                                value[0] * value[0],
                                two * value[0] * value[1],
                                (two * value[0] * value[2]) +
                                  (value[1] * value[1]));

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        sigma = time_domain<time>::infinity;

        if (have_message(x[0])) {
            auto  lst = get_message(sim, x[0]);
            auto& msg = lst.front();

            if constexpr (QssLevel == 1) {
                value[0] = msg[0];
            }

            if constexpr (QssLevel == 2) {
                value[0] = msg[0];
                value[1] = msg[1];
            }

            if constexpr (QssLevel == 3) {
                value[0] = msg[0];
                value[1] = msg[1];
                value[2] = msg[2];
            }

            sigma = time_domain<time>::zero;
        }

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { value[0] };
    }
};

using qss1_square = abstract_square<1>;
using qss2_square = abstract_square<2>;
using qss3_square = abstract_square<3>;

template<int QssLevel, int PortNumber>
struct abstract_sum
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");
    static_assert(PortNumber > 1, "sum model need at least two input port");

    input_port  x[PortNumber];
    output_port y[1];
    time        sigma;

    real values[QssLevel * PortNumber];

    abstract_sum() noexcept = default;

    abstract_sum(const abstract_sum& other) noexcept
      : sigma(other.sigma)
    {
        std::copy_n(other.values, QssLevel * PortNumber, values);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(values, QssLevel * PortNumber, zero);
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1) {
            real value = 0.;
            for (int i = 0; i != PortNumber; ++i)
                value += values[i];

            return send_message(sim, y[0], value);
        }

        if constexpr (QssLevel == 2) {
            real value = 0.;
            real slope = 0.;

            for (int i = 0; i != PortNumber; ++i) {
                value += values[i];
                slope += values[i + PortNumber];
            }

            return send_message(sim, y[0], value, slope);
        }

        if constexpr (QssLevel == 3) {
            real value      = 0.;
            real slope      = 0.;
            real derivative = 0.;

            for (size_t i = 0; i != PortNumber; ++i) {
                value += values[i];
                slope += values[i + PortNumber];
                derivative += values[i + PortNumber + PortNumber];
            }

            return send_message(sim, y[0], value, slope, derivative);
        }

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      [[maybe_unused]] time e,
                      time /*r*/) noexcept
    {
        bool message = false;

        if constexpr (QssLevel == 1) {
            for (size_t i = 0; i != PortNumber; ++i) {
                auto span = get_message(sim, x[i]);
                for (const auto& msg : span) {
                    values[i] = msg[0];
                    message   = true;
                }
            }
        }

        if constexpr (QssLevel == 2) {
            for (size_t i = 0; i != PortNumber; ++i) {
                if (!have_message(x[i])) {
                    values[i] += values[i + PortNumber] * e;
                } else {
                    auto span = get_message(sim, x[i]);
                    for (const auto& msg : span) {
                        values[i]              = msg[0];
                        values[i + PortNumber] = msg[1];
                        message                = true;
                    }
                }
            }
        }

        if constexpr (QssLevel == 3) {
            for (size_t i = 0; i != PortNumber; ++i) {
                if (!have_message(x[i])) {
                    values[i] += values[i + PortNumber] * e +
                                 values[i + PortNumber + PortNumber] * e * e;
                    values[i + PortNumber] +=
                      2 * values[i + PortNumber + PortNumber] * e;
                } else {
                    auto span = get_message(sim, x[i]);
                    for (const auto& msg : span) {
                        values[i]                           = msg[0];
                        values[i + PortNumber]              = msg[1];
                        values[i + PortNumber + PortNumber] = msg[2];
                        message                             = true;
                    }
                }
            }
        }

        sigma = message ? time_domain<time>::zero : time_domain<time>::infinity;

        return status::success;
    }

    observation_message observation(
      [[maybe_unused]] const time e) const noexcept
    {
        real value = 0.;

        if constexpr (QssLevel == 1) {
            for (size_t i = 0; i != PortNumber; ++i)
                value += values[i];
        }

        if constexpr (QssLevel >= 2) {
            for (size_t i = 0; i != PortNumber; ++i)
                value += values[i + PortNumber] * e;
        }

        if constexpr (QssLevel >= 3) {
            for (size_t i = 0; i != PortNumber; ++i)
                value += values[i + PortNumber + PortNumber] * e * e;
        }

        return { value };
    }
};

using qss1_sum_2 = abstract_sum<1, 2>;
using qss1_sum_3 = abstract_sum<1, 3>;
using qss1_sum_4 = abstract_sum<1, 4>;
using qss2_sum_2 = abstract_sum<2, 2>;
using qss2_sum_3 = abstract_sum<2, 3>;
using qss2_sum_4 = abstract_sum<2, 4>;
using qss3_sum_2 = abstract_sum<3, 2>;
using qss3_sum_3 = abstract_sum<3, 3>;
using qss3_sum_4 = abstract_sum<3, 4>;

template<int QssLevel, int PortNumber>
struct abstract_wsum
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");
    static_assert(PortNumber > 1, "sum model need at least two input port");

    input_port  x[PortNumber];
    output_port y[1];
    time        sigma;

    real default_input_coeffs[PortNumber] = { 0 };
    real values[QssLevel * PortNumber];

    abstract_wsum() noexcept = default;

    abstract_wsum(const abstract_wsum& other) noexcept
      : sigma(other.sigma)
    {
        std::copy_n(
          other.default_input_coeffs, PortNumber, default_input_coeffs);
        std::copy_n(other.values, QssLevel * PortNumber, values);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(values, QssLevel * PortNumber, zero);
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1) {
            real value = zero;

            for (int i = 0; i != PortNumber; ++i)
                value += default_input_coeffs[i] * values[i];

            return send_message(sim, y[0], value);
        }

        if constexpr (QssLevel == 2) {
            real value = zero;
            real slope = zero;

            for (int i = 0; i != PortNumber; ++i) {
                value += default_input_coeffs[i] * values[i];
                slope += default_input_coeffs[i] * values[i + PortNumber];
            }

            return send_message(sim, y[0], value, slope);
        }

        if constexpr (QssLevel == 3) {
            real value      = zero;
            real slope      = zero;
            real derivative = zero;

            for (int i = 0; i != PortNumber; ++i) {
                value += default_input_coeffs[i] * values[i];
                slope += default_input_coeffs[i] * values[i + PortNumber];
                derivative +=
                  default_input_coeffs[i] * values[i + PortNumber + PortNumber];
            }

            return send_message(sim, y[0], value, slope, derivative);
        }

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      [[maybe_unused]] time e,
                      time /*r*/) noexcept
    {
        bool message = false;

        if constexpr (QssLevel == 1) {
            for (size_t i = 0; i != PortNumber; ++i) {
                auto span = get_message(sim, x[i]);
                for (const auto& msg : span) {
                    values[i] = msg[0];
                    message   = true;
                }
            }
        }

        if constexpr (QssLevel == 2) {
            for (size_t i = 0; i != PortNumber; ++i) {
                if (!have_message(x[i])) {
                    values[i] += values[i + PortNumber] * e;
                } else {
                    auto span = get_message(sim, x[i]);
                    for (const auto& msg : span) {
                        values[i]              = msg[0];
                        values[i + PortNumber] = msg[1];
                        message                = true;
                    }
                }
            }
        }

        if constexpr (QssLevel == 3) {
            for (size_t i = 0; i != PortNumber; ++i) {
                if (!have_message(x[i])) {
                    values[i] += values[i + PortNumber] * e +
                                 values[i + PortNumber + PortNumber] * e * e;
                    values[i + PortNumber] +=
                      2 * values[i + PortNumber + PortNumber] * e;
                } else {
                    auto span = get_message(sim, x[i]);
                    for (const auto& msg : span) {
                        values[i]                           = msg[0];
                        values[i + PortNumber]              = msg[1];
                        values[i + PortNumber + PortNumber] = msg[2];
                        message                             = true;
                    }
                }
            }
        }

        sigma = message ? time_domain<time>::zero : time_domain<time>::infinity;

        return status::success;
    }

    observation_message observation(
      [[maybe_unused]] const time e) const noexcept
    {
        real value = zero;

        if constexpr (QssLevel >= 1) {
            for (int i = 0; i != PortNumber; ++i)
                value += default_input_coeffs[i] * values[i];
        }

        if constexpr (QssLevel >= 2) {
            for (int i = 0; i != PortNumber; ++i)
                value += default_input_coeffs[i] * values[i + PortNumber] * e;
        }

        if constexpr (QssLevel >= 3) {
            for (size_t i = 0; i != PortNumber; ++i)
                value += default_input_coeffs[i] *
                         values[i + PortNumber + PortNumber] * e * e;
        }

        return { value };
    }
};

using qss1_wsum_2 = abstract_wsum<1, 2>;
using qss1_wsum_3 = abstract_wsum<1, 3>;
using qss1_wsum_4 = abstract_wsum<1, 4>;
using qss2_wsum_2 = abstract_wsum<2, 2>;
using qss2_wsum_3 = abstract_wsum<2, 3>;
using qss2_wsum_4 = abstract_wsum<2, 4>;
using qss3_wsum_2 = abstract_wsum<3, 2>;
using qss3_wsum_3 = abstract_wsum<3, 3>;
using qss3_wsum_4 = abstract_wsum<3, 4>;

template<int QssLevel>
struct abstract_multiplier
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");

    input_port  x[2];
    output_port y[1];
    time        sigma;

    real values[QssLevel * 2];

    abstract_multiplier() noexcept = default;

    abstract_multiplier(const abstract_multiplier& other) noexcept
      : sigma(other.sigma)
    {
        std::copy_n(other.values, std::size(values), values);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(values, QssLevel * 2, zero);
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1) {
            return send_message(sim, y[0], values[0] * values[1]);
        }

        if constexpr (QssLevel == 2) {
            return send_message(sim,
                                y[0],
                                values[0] * values[1],
                                values[2 + 0] * values[1] +
                                  values[2 + 1] * values[0]);
        }

        if constexpr (QssLevel == 3) {
            return send_message(
              sim,
              y[0],
              values[0] * values[1],
              values[2 + 0] * values[1] + values[2 + 1] * values[0],
              values[0] * values[2 + 2 + 1] + values[2 + 0] * values[2 + 1] +
                values[2 + 2 + 0] * values[1]);
        }

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      [[maybe_unused]] time e,
                      time /*r*/) noexcept
    {
        bool message_port_0 = have_message(x[0]);
        bool message_port_1 = have_message(x[1]);
        sigma               = time_domain<time>::infinity;
        auto span_0         = get_message(sim, x[0]);
        auto span_1         = get_message(sim, x[1]);

        for (const auto& msg : span_0) {
            sigma     = time_domain<time>::zero;
            values[0] = msg[0];

            if constexpr (QssLevel >= 2)
                values[2 + 0] = msg[1];

            if constexpr (QssLevel == 3)
                values[2 + 2 + 0] = msg[2];
        }

        for (const auto& msg : span_1) {
            sigma     = time_domain<time>::zero;
            values[1] = msg[0];

            if constexpr (QssLevel >= 2)
                values[2 + 1] = msg[1];

            if constexpr (QssLevel == 3)
                values[2 + 2 + 1] = msg[2];
        }

        if constexpr (QssLevel == 2) {
            if (!message_port_0)
                values[0] += e * values[2 + 0];

            if (!message_port_1)
                values[1] += e * values[2 + 1];
        }

        if constexpr (QssLevel == 3) {
            if (!message_port_0) {
                values[0] += e * values[2 + 0] + values[2 + 2 + 0] * e * e;
                values[2 + 0] += 2 * values[2 + 2 + 0] * e;
            }

            if (!message_port_1) {
                values[1] += e * values[2 + 1] + values[2 + 2 + 1] * e * e;
                values[2 + 1] += 2 * values[2 + 2 + 1] * e;
            }
        }

        return status::success;
    }

    observation_message observation(
      [[maybe_unused]] const time e) const noexcept
    {
        if constexpr (QssLevel == 1)
            return { values[0] * values[1] };

        if constexpr (QssLevel == 2)
            return { (values[0] + e * values[2 + 0]) *
                     (values[1] + e * values[2 + 1]) };

        if constexpr (QssLevel == 3)
            return {
                (values[0] + e * values[2 + 0] + e * e * values[2 + 2 + 0]) *
                (values[1] + e * values[2 + 1] + e * e * values[2 + 2 + 1])
            };
    }
};

using qss1_multiplier = abstract_multiplier<1>;
using qss2_multiplier = abstract_multiplier<2>;
using qss3_multiplier = abstract_multiplier<3>;

struct quantifier
{
    input_port  x[1];
    output_port y[1];
    time        sigma = time_domain<time>::infinity;

    enum class state
    {
        init,
        idle,
        response
    };

    enum class adapt_state
    {
        impossible,
        possible,
        done
    };

    enum class direction
    {
        up,
        down
    };

    real        default_step_size        = real(0.001);
    int         default_past_length      = 3;
    adapt_state default_adapt_state      = adapt_state::possible;
    bool        default_zero_init_offset = false;
    u64         archive                  = -1;
    int         archive_length           = 0;

    real        m_upthreshold      = zero;
    real        m_downthreshold    = zero;
    real        m_offset           = zero;
    real        m_step_size        = zero;
    int         m_step_number      = 0;
    int         m_past_length      = 0;
    bool        m_zero_init_offset = false;
    state       m_state            = state::init;
    adapt_state m_adapt_state      = adapt_state::possible;

    quantifier() noexcept = default;

    quantifier(const quantifier& other) noexcept
      : default_step_size(other.default_step_size)
      , default_past_length(other.default_past_length)
      , default_adapt_state(other.default_adapt_state)
      , default_zero_init_offset(other.default_zero_init_offset)
      , archive(-1)
      , archive_length(0)
      , m_upthreshold(other.m_upthreshold)
      , m_downthreshold(other.m_downthreshold)
      , m_offset(other.m_offset)
      , m_step_size(other.m_step_size)
      , m_step_number(other.m_step_number)
      , m_past_length(other.m_past_length)
      , m_zero_init_offset(other.m_zero_init_offset)
      , m_state(other.m_state)
      , m_adapt_state(other.m_adapt_state)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        m_step_size        = default_step_size;
        m_past_length      = default_past_length;
        m_zero_init_offset = default_zero_init_offset;
        m_adapt_state      = default_adapt_state;
        m_upthreshold      = zero;
        m_downthreshold    = zero;
        m_offset           = zero;
        m_step_number      = 0;
        archive            = -1;
        archive_length     = 0;
        m_state            = state::init;

        irt_return_if_fail(m_step_size > 0,
                           status::model_quantifier_bad_quantum_parameter);

        irt_return_if_fail(
          m_past_length > 2,
          status::model_quantifier_bad_archive_length_parameter);

        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        append_archive(sim, archive).clear();
        return status::success;
    }

    status external(simulation& sim, time t) noexcept
    {
        real val = 0.0, shifting_factor = 0.0;

        {
            auto span = get_message(sim, x[0]);
            real sum  = zero;
            real nb   = zero;

            for (const auto& elem : span) {
                sum += elem[0];
                ++nb;
            }

            val = sum / nb;
        }

        if (m_state == state::init) {
            init_step_number_and_offset(val);
            update_thresholds();
            m_state = state::response;
            return status::success;
        }

        while ((val >= m_upthreshold) || (val <= m_downthreshold)) {
            m_step_number =
              val >= m_upthreshold ? m_step_number + 1 : m_step_number - 1;

            switch (m_adapt_state) {
            case adapt_state::impossible:
                update_thresholds();
                break;
            case adapt_state::possible:
                store_change(
                  sim, val >= m_upthreshold ? m_step_size : -m_step_size, t);

                shifting_factor = shift_quanta(sim);

                irt_return_if_fail(shifting_factor >= 0,
                                   status::model_quantifier_shifting_value_neg);
                irt_return_if_fail(
                  shifting_factor <= 1,
                  status::model_quantifier_shifting_value_less_1);

                if ((0 != shifting_factor) && (1 != shifting_factor)) {
                    update_thresholds(shifting_factor,
                                      val >= m_upthreshold ? direction::down
                                                           : direction::up);
                    m_adapt_state = adapt_state::done;
                } else {
                    update_thresholds();
                }
                break;
            case adapt_state::done:
                init_step_number_and_offset(val);
                m_adapt_state = adapt_state::possible;
                update_thresholds();
                break;
            }
        }

        m_state = state::response;
        return status::success;
    }

    status internal() noexcept
    {
        if (m_state == state::response)
            m_state = state::idle;

        return status::success;
    }

    status transition(simulation& sim, time t, time /*e*/, time r) noexcept
    {
        if (!have_message(x[0])) {
            irt_return_if_bad(internal());
        } else {
            if (time_domain<time>::is_zero(r))
                irt_return_if_bad(internal());

            irt_return_if_bad(external(sim, t));
        }

        return ta();
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], m_upthreshold, m_downthreshold);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { m_upthreshold, m_downthreshold };
    }

private:
    status ta() noexcept
    {
        sigma = m_state == state::response ? time_domain<time>::zero
                                           : time_domain<time>::infinity;

        return status::success;
    }

    void update_thresholds() noexcept
    {
        const auto step_number = static_cast<real>(m_step_number);

        m_upthreshold   = m_offset + m_step_size * (step_number + one);
        m_downthreshold = m_offset + m_step_size * (step_number - one);
    }

    void update_thresholds(real factor) noexcept
    {
        const auto step_number = static_cast<real>(m_step_number);

        m_upthreshold = m_offset + m_step_size * (step_number + (one - factor));
        m_downthreshold =
          m_offset + m_step_size * (step_number - (one - factor));
    }

    void update_thresholds(real factor, direction d) noexcept
    {
        const auto step_number = static_cast<real>(m_step_number);

        if (d == direction::up) {
            m_upthreshold =
              m_offset + m_step_size * (step_number + (one - factor));
            m_downthreshold = m_offset + m_step_size * (step_number - one);
        } else {
            m_upthreshold = m_offset + m_step_size * (step_number + one);
            m_downthreshold =
              m_offset + m_step_size * (step_number - (one - factor));
        }
    }

    void init_step_number_and_offset(real value) noexcept
    {
        m_step_number = static_cast<int>(std::floor(value / m_step_size));

        if (m_zero_init_offset) {
            m_offset = 0.0;
        } else {
            m_offset = value - static_cast<real>(m_step_number) * m_step_size;
        }
    }

    real shift_quanta(simulation& sim)
    {
        auto lst    = append_archive(sim, archive);
        real factor = 0.0;

        if (oscillating(sim, m_past_length - 1) &&
            ((lst.back().date - lst.front().date) != 0)) {
            real acc = 0.0;
            real local_estim;
            real cnt = 0;

            auto it_2 = lst.begin();
            auto it_0 = it_2++;
            auto it_1 = it_2++;

            for (int i = 0; i < archive_length - 2; ++i) {
                if ((it_2->date - it_0->date) != 0) {
                    if ((lst.back().x_dot * it_1->x_dot) > zero) {
                        local_estim = 1 - (it_1->date - it_0->date) /
                                            (it_2->date - it_0->date);
                    } else {
                        local_estim =
                          (it_1->date - it_0->date) / (it_2->date - it_0->date);
                    }

                    acc += local_estim;
                    cnt += one;
                }
            }

            acc    = acc / cnt;
            factor = acc;
            lst.clear();
            archive_length = 0;
        }

        return factor;
    }

    void store_change(simulation& sim, real val, time t) noexcept
    {
        auto lst = append_archive(sim, archive);
        lst.emplace_back(val, t);
        archive_length++;

        while (archive_length > m_past_length) {
            lst.pop_front();
            archive_length--;
        }
    }

    bool oscillating(simulation& sim, const int range) noexcept
    {
        if ((range + 1) > archive_length)
            return false;

        auto      lst   = get_archive(sim, archive);
        const int limit = archive_length - range;
        auto      it    = lst.end();
        --it;
        auto next = it--;

        for (int i = 0; i < limit; ++i, next = it--)
            if (it->x_dot * next->x_dot > 0)
                return false;

        return true;
    }

    bool monotonous(simulation& sim, const int range) noexcept
    {
        if ((range + 1) > archive_length)
            return false;

        auto lst  = get_archive(sim, archive);
        auto it   = lst.begin();
        auto prev = it++;

        for (int i = 0; i < range; ++i, prev = it++)
            if ((prev->x_dot * it->x_dot) < 0)
                return false;

        return true;
    }
};

template<size_t PortNumber>
struct adder
{
    static_assert(PortNumber > 1, "adder model need at least two input port");

    input_port  x[PortNumber];
    output_port y[1];
    time        sigma;

    real default_values[PortNumber];
    real default_input_coeffs[PortNumber];

    real values[PortNumber];
    real input_coeffs[PortNumber];

    adder() noexcept
    {
        constexpr auto div = one / static_cast<real>(PortNumber);

        std::fill_n(std::begin(default_values), PortNumber, div);
        std::fill_n(std::begin(default_input_coeffs), PortNumber, zero);
    }

    adder(const adder& other) noexcept
      : sigma(other.sigma)
    {
        std::copy_n(other.default_values,
                    std::size(other.default_values),
                    default_values);
        std::copy_n(other.default_input_coeffs,
                    std::size(other.default_input_coeffs),
                    default_input_coeffs);
        std::copy_n(other.values, std::size(other.values), values);
        std::copy_n(
          other.input_coeffs, std::size(other.input_coeffs), input_coeffs);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::copy_n(std::begin(default_values), PortNumber, std::begin(values));

        std::copy_n(std::begin(default_input_coeffs),
                    PortNumber,
                    std::begin(input_coeffs));

        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        real to_send = zero;

        for (size_t i = 0; i != PortNumber; ++i)
            to_send += input_coeffs[i] * values[i];

        return send_message(sim, y[0], to_send);
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        bool have_message = false;

        for (size_t i = 0; i != PortNumber; ++i) {
            auto span = get_message(sim, x[i]);
            for (const auto& msg : span) {
                values[i] = msg[0];

                have_message = true;
            }
        }

        sigma =
          have_message ? time_domain<time>::zero : time_domain<time>::infinity;

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        real ret = zero;

        for (size_t i = 0; i != PortNumber; ++i)
            ret += input_coeffs[i] * values[i];

        return { ret };
    }
};

template<size_t PortNumber>
struct mult
{
    static_assert(PortNumber > 1, "mult model need at least two input port");

    input_port  x[PortNumber];
    output_port y[1];
    time        sigma;

    real default_values[PortNumber];
    real default_input_coeffs[PortNumber];

    real values[PortNumber];
    real input_coeffs[PortNumber];

    mult() noexcept
    {
        std::fill_n(std::begin(default_values), PortNumber, one);
        std::fill_n(std::begin(default_input_coeffs), PortNumber, zero);
    }

    mult(const mult& other) noexcept
    {
        std::copy_n(other.default_values,
                    std::size(other.default_values),
                    default_values);
        std::copy_n(other.default_input_coeffs,
                    std::size(other.default_input_coeffs),
                    default_input_coeffs);
        std::copy_n(other.values, std::size(other.values), values);
        std::copy_n(
          other.input_coeffs, std::size(other.input_coeffs), input_coeffs);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        std::copy_n(std::begin(default_values), PortNumber, std::begin(values));

        std::copy_n(std::begin(default_input_coeffs),
                    PortNumber,
                    std::begin(input_coeffs));

        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        real to_send = 1.0;

        for (size_t i = 0; i != PortNumber; ++i)
            to_send *= std::pow(values[i], input_coeffs[i]);

        return send_message(sim, y[0], to_send);
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        bool have_message = false;
        for (size_t i = 0; i != PortNumber; ++i) {
            auto span = get_message(sim, x[i]);
            for (const auto& msg : span) {
                values[i]    = msg[0];
                have_message = true;
            }
        }

        sigma =
          have_message ? time_domain<time>::zero : time_domain<time>::infinity;

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        real ret = 1.0;

        for (size_t i = 0; i != PortNumber; ++i)
            ret *= std::pow(values[i], input_coeffs[i]);

        return { ret };
    }
};

struct counter
{
    input_port x[1];
    time       sigma;
    i64        number;

    counter() noexcept = default;

    counter(const counter& other) noexcept
      : sigma(other.sigma)
      , number(other.number)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        number = { 0 };
        sigma  = time_domain<time>::infinity;

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        auto list = get_message(sim, x[0]);
        auto it   = list.begin();
        auto end  = list.end();

        for (; it != end; ++it)
            ++number;

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { static_cast<real>(number) };
    }
};

struct generator
{
    output_port y[1];
    time        sigma;
    real        value;

    real   default_offset = 0.0;
    source default_source_ta;
    source default_source_value;
    bool   stop_on_error = false;

    generator() noexcept = default;

    generator(const generator& other) noexcept
      : sigma(other.sigma)
      , value(other.value)
      , default_offset(other.default_offset)
      , default_source_ta(other.default_source_ta)
      , default_source_value(other.default_source_value)
      , stop_on_error(other.stop_on_error)
    {}

    status initialize(simulation& sim) noexcept
    {
        sigma = default_offset;

        if (stop_on_error) {
            irt_return_if_bad(initialize_source(sim, default_source_ta));
            irt_return_if_bad(initialize_source(sim, default_source_value));
        } else {
            (void)initialize_source(sim, default_source_ta);
            (void)initialize_source(sim, default_source_value);
        }

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        irt_return_if_bad(finalize_source(sim, default_source_ta));
        irt_return_if_bad(finalize_source(sim, default_source_value));

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        double local_sigma = 0;
        double local_value = 0;

        if (stop_on_error) {
            irt_return_if_bad(
              update_source(sim, default_source_ta, local_sigma));
            irt_return_if_bad(
              update_source(sim, default_source_value, local_value));
            sigma = static_cast<real>(local_sigma);
            value = static_cast<real>(local_value);
        } else {
            if (is_bad(update_source(sim, default_source_ta, local_sigma)))
                sigma = time_domain<time>::infinity;
            else
                sigma = static_cast<real>(local_sigma);

            if (is_bad(update_source(sim, default_source_value, local_value)))
                value = 0;
            else
                value = static_cast<real>(local_value);
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], value);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { value };
    }
};

struct constant
{
    output_port y[1];
    time        sigma;

    real default_value  = 0.0;
    time default_offset = time_domain<time>::zero;

    real value = 0.0;

    constant() noexcept = default;

    constant(const constant& other) noexcept
      : sigma(other.sigma)
      , default_value(other.default_value)
      , default_offset(other.default_offset)
      , value(other.value)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        sigma = default_offset;

        value = default_value;

        return status::success;
    }

    status transition(simulation& /*sim*/,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], value);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { value };
    }
};

struct filter
{
    input_port  x[1];
    output_port y[1];
    time        sigma;

    real default_lower_threshold;
    real default_upper_threshold;

    real         lower_threshold;
    real         upper_threshold;
    irt::message inValue;

    filter() noexcept
    {
        default_lower_threshold = -0.5;
        default_upper_threshold = 0.5;
        sigma                   = time_domain<time>::infinity;
    }

    status initialize() noexcept
    {
        sigma           = time_domain<time>::infinity;
        lower_threshold = default_lower_threshold;
        upper_threshold = default_upper_threshold;

        irt_return_if_fail(default_lower_threshold < default_upper_threshold,
                           status::filter_threshold_condition_not_satisfied);

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], inValue[0]);
    }

    status transition(simulation& sim, time, time, time) noexcept
    {
        sigma = time_domain<time>::infinity;

        auto span = get_message(sim, x[0]);
        for (const auto& msg : span) {
            if (msg[0] > lower_threshold && msg[0] < upper_threshold) {
                inValue[0] = msg[0];
            } else if (msg[1] < lower_threshold && msg[1] < upper_threshold) {
                inValue[0] = msg[1];
            } else {
                inValue[0] = msg[2];
            }

            sigma = time_domain<time>::zero;
        }

        return status::success;
    }

    message observation(const time) const noexcept { return inValue[0]; }
};

struct flow
{
    output_port y[1];
    time        sigma;

    real  default_samplerate = 44100.0;
    real* default_data       = nullptr;
    real* default_sigmas     = nullptr;
    sz    default_size       = 0u;

    real accu_sigma;
    sz   i;

    flow() noexcept = default;

    flow(const flow& other) noexcept
      : sigma(other.sigma)
      , default_samplerate(other.default_samplerate)
      , default_data(other.default_data)
      , default_sigmas(other.default_sigmas)
      , default_size(other.default_size)
      , accu_sigma(other.accu_sigma)
      , i(other.i)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        irt_return_if_fail(default_samplerate > zero,
                           status::model_flow_bad_samplerate);

        irt_return_if_fail(default_data != nullptr &&
                             default_sigmas != nullptr && default_size > 1,
                           status::model_flow_bad_data);

        sigma      = one / default_samplerate;
        accu_sigma = zero;
        i          = 0;

        return status::success;
    }

    status transition(simulation& /*sim*/,
                      time t,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        for (; i < default_size; ++i) {
            accu_sigma += default_sigmas[i];

            if (accu_sigma > t) {
                sigma = default_sigmas[i];
                return status::success;
            }
        }

        sigma = time_domain<time>::infinity;
        i     = default_size - 1;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], default_data[i]);
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { default_data[i] };
    }
};

template<size_t PortNumber>
struct accumulator
{
    input_port x[2 * PortNumber];
    time       sigma;
    real       number;
    real       numbers[PortNumber];

    accumulator() = default;

    accumulator(const accumulator& other) noexcept
      : sigma(other.sigma)
      , number(other.number)
    {
        std::copy_n(other.numbers, std::size(numbers), numbers);
    }

    status initialize(simulation& /*sim*/) noexcept
    {
        number = zero;
        std::fill_n(numbers, PortNumber, zero);

        sigma = time_domain<time>::infinity;

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        for (sz i = 0; i != PortNumber; ++i) {
            if (have_message(x[i + PortNumber])) {
                auto list  = get_message(sim, x[i + PortNumber]);
                numbers[i] = list.front()[0];
            }
        }

        for (sz i = 0; i != PortNumber; ++i) {
            if (have_message(x[i])) {
                auto list = get_message(sim, x[i]);
                if (list.front()[0] != zero)
                    number += numbers[i];
            }
        }

        return status::success;
    }
};

struct cross
{
    input_port  x[4];
    output_port y[2];
    time        sigma;

    real default_threshold = zero;

    real threshold;
    real value;
    real if_value;
    real else_value;
    real result;
    real event;

    cross() noexcept = default;

    cross(const cross& other) noexcept
      : sigma(other.sigma)
      , default_threshold(other.default_threshold)
      , threshold(other.threshold)
      , value(other.value)
      , if_value(other.if_value)
      , else_value(other.else_value)
      , result(other.result)
      , event(other.event)
    {}

    enum port_name
    {
        port_value,
        port_if_value,
        port_else_value,
        port_threshold
    };

    status initialize(simulation& /*sim*/) noexcept
    {
        threshold  = default_threshold;
        value      = threshold - one;
        if_value   = zero;
        else_value = zero;
        result     = zero;
        event      = zero;

        sigma = time_domain<time>::zero;

        return status::success;
    }

    status transition(simulation& sim,
                      time /*t*/,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        bool have_message       = false;
        bool have_message_value = false;
        event                   = zero;

        auto span_thresholds = get_message(sim, x[port_threshold]);
        for (auto& elem : span_thresholds) {
            threshold    = elem[0];
            have_message = true;
        }

        auto span_value = get_message(sim, x[port_value]);
        for (auto& elem : span_value) {
            value              = elem[0];
            have_message_value = true;
            have_message       = true;
        }

        auto span_if_value = get_message(sim, x[port_if_value]);
        for (auto& elem : span_if_value) {
            if_value     = elem[0];
            have_message = true;
        }

        auto span_else_value = get_message(sim, x[port_else_value]);
        for (auto& elem : span_else_value) {
            else_value   = elem[0];
            have_message = true;
        }

        if (have_message_value) {
            event = zero;
            if (value >= threshold) {
                else_value = if_value;
                event      = one;
            }
        }

        result = else_value;

        sigma =
          have_message ? time_domain<time>::zero : time_domain<time>::infinity;

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        irt_return_if_bad(send_message(sim, y[0], result));
        irt_return_if_bad(send_message(sim, y[1], event));

        return status::success;
    }

    observation_message observation(const time /*e*/) const noexcept
    {
        return { value, if_value, else_value };
    }
};

template<int QssLevel>
struct abstract_cross
{
    static_assert(1 <= QssLevel && QssLevel <= 3, "Only for Qss1, 2 and 3");

    input_port  x[4];
    output_port y[3];
    time        sigma;

    real default_threshold = zero;
    bool default_detect_up = true;

    real threshold;
    real if_value[QssLevel];
    real else_value[QssLevel];
    real value[QssLevel];
    real last_reset;
    bool reach_threshold;
    bool detect_up;

    abstract_cross() noexcept = default;

    abstract_cross(const abstract_cross& other) noexcept
      : sigma(other.sigma)
      , default_threshold(other.default_threshold)
      , default_detect_up(other.default_detect_up)
      , threshold(other.threshold)
      , last_reset(other.last_reset)
      , reach_threshold(other.reach_threshold)
      , detect_up(other.detect_up)
    {
        std::copy_n(other.if_value, QssLevel, if_value);
        std::copy_n(other.else_value, QssLevel, else_value);
        std::copy_n(other.value, QssLevel, value);
    }

    enum port_name
    {
        port_value,
        port_if_value,
        port_else_value,
        port_threshold
    };

    enum out_name
    {
        o_if_value,
        o_else_value,
        o_event
    };

    status initialize(simulation& /*sim*/) noexcept
    {
        std::fill_n(if_value, QssLevel, zero);
        std::fill_n(else_value, QssLevel, zero);
        std::fill_n(value, QssLevel, zero);

        threshold = default_threshold;
        value[0]  = threshold - one;

        sigma           = time_domain<time>::infinity;
        last_reset      = time_domain<time>::infinity;
        detect_up       = default_detect_up;
        reach_threshold = false;

        return status::success;
    }

    void compute_wake_up() noexcept
    {
        if constexpr (QssLevel == 1) {
            sigma = time_domain<time>::infinity;
        }

        if constexpr (QssLevel == 2) {
            sigma = time_domain<time>::infinity;
            if (value[1]) {
                const auto a = value[1];
                const auto b = value[0] - threshold;
                const auto d = -b * a;

                if (d > zero)
                    sigma = d;
            }
        }

        if constexpr (QssLevel == 3) {
            sigma = time_domain<time>::infinity;
            if (value[1]) {
                if (value[2]) {
                    const auto a = value[2];
                    const auto b = value[1];
                    const auto c = value[0] - threshold;
                    const auto d = b * b - four * a * c;

                    if (d > zero) {
                        const auto x1 = (-b + std::sqrt(d)) / (two * a);
                        const auto x2 = (-b - std::sqrt(d)) / (two * a);

                        if (x1 > zero) {
                            if (x2 > zero) {
                                sigma = std::min(x1, x2);
                            } else {
                                sigma = x1;
                            }
                        } else {
                            if (x2 > 0)
                                sigma = x2;
                        }
                    }
                    if (d == zero) {
                        const auto x = -b / (two * a);
                        if (x > zero)
                            sigma = x;
                    }
                } else {
                    const auto a = value[1];
                    const auto b = value[0] - threshold;
                    const auto d = -b * a;

                    if (d > zero)
                        sigma = d;
                }
            }
        }
    }

    status transition(simulation&           sim,
                      time                  t,
                      [[maybe_unused]] time e,
                      time /*r*/) noexcept
    {
        const auto old_else_value = else_value[0];

        if (have_message(x[port_threshold])) {
            auto span = get_message(sim, x[port_threshold]);
            for (const auto& msg : span)
                threshold = msg[0];
        }

        if (!have_message(x[port_if_value])) {
            if constexpr (QssLevel == 2)
                if_value[0] += if_value[1] * e;
            if constexpr (QssLevel == 3) {
                if_value[0] += if_value[1] * e + if_value[2] * e * e;
                if_value[1] += two * if_value[2] * e;
            }
        } else {
            auto span = get_message(sim, x[port_if_value]);
            for (const auto& msg : span) {
                if_value[0] = msg[0];
                if constexpr (QssLevel >= 2)
                    if_value[1] = msg[1];
                if constexpr (QssLevel == 3)
                    if_value[2] = msg[2];
            }
        }

        if (!have_message(x[port_else_value])) {
            if constexpr (QssLevel == 2)
                else_value[0] += else_value[1] * e;
            if constexpr (QssLevel == 3) {
                else_value[0] += else_value[1] * e + else_value[2] * e * e;
                else_value[1] += two * else_value[2] * e;
            }
        } else {
            auto span = get_message(sim, x[port_else_value]);
            for (const auto& msg : span) {
                else_value[0] = msg[0];
                if constexpr (QssLevel >= 2)
                    else_value[1] = msg[1];
                if constexpr (QssLevel == 3)
                    else_value[2] = msg[2];
            }
        }

        if (!have_message(x[port_value])) {
            if constexpr (QssLevel == 2)
                value[0] += value[1] * e;
            if constexpr (QssLevel == 3) {
                value[0] += value[1] * e + value[2] * e * e;
                value[1] += two * value[2] * e;
            }
        } else {
            auto span = get_message(sim, x[port_value]);
            for (const auto& msg : span) {
                value[0] = msg[0];
                if constexpr (QssLevel >= 2)
                    value[1] = msg[1];
                if constexpr (QssLevel == 3)
                    value[2] = msg[2];
            }
        }

        reach_threshold = false;

        if ((detect_up && value[0] >= threshold) ||
            (!detect_up && value[0] <= threshold)) {
            if (t != last_reset) {
                last_reset      = t;
                reach_threshold = true;
                sigma           = time_domain<time>::zero;
            } else
                sigma = time_domain<time>::infinity;
        } else if (old_else_value != else_value[0]) {
            sigma = time_domain<time>::zero;
        } else {
            compute_wake_up();
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if constexpr (QssLevel == 1) {
            irt_return_if_bad(
              send_message(sim, y[o_else_value], else_value[0]));

            if (reach_threshold) {
                irt_return_if_bad(
                  send_message(sim, y[o_if_value], if_value[0]));
                irt_return_if_bad(send_message(sim, y[o_event], one));
            }

            return status::success;
        }

        if constexpr (QssLevel == 2) {
            irt_return_if_bad(
              send_message(sim, y[o_else_value], else_value[0], else_value[1]));

            if (reach_threshold) {
                irt_return_if_bad(
                  send_message(sim, y[o_if_value], if_value[0], if_value[1]));
                irt_return_if_bad(send_message(sim, y[o_event], one));
            }

            return status::success;
        }

        if constexpr (QssLevel == 3) {
            irt_return_if_bad(send_message(sim,
                                           y[o_else_value],
                                           else_value[0],
                                           else_value[1],
                                           else_value[2]));

            if (reach_threshold) {
                irt_return_if_bad(send_message(
                  sim, y[o_if_value], if_value[0], if_value[1], if_value[2]));
                irt_return_if_bad(send_message(sim, y[o_event], one));
            }

            return status::success;
        }

        return status::success;
    }

    observation_message observation(const time /*t*/) const noexcept
    {
        return { value[0], if_value[0], else_value[0] };
    }
};

using qss1_cross = abstract_cross<1>;
using qss2_cross = abstract_cross<2>;
using qss3_cross = abstract_cross<3>;

inline real sin_time_function(real t) noexcept
{
    constexpr real f0 = to_real(0.1);

#if irt_have_numbers == 1
    constexpr real pi = std::numbers::pi_v<real>;
#else
    // std::acos(-1) is not a constexpr in MVSC 2019
    constexpr real pi = 3.141592653589793238462643383279502884;
#endif

    constexpr const real mult = two * pi * f0;

    return std::sin(mult * t);
}

inline real square_time_function(real t) noexcept { return t * t; }

inline real time_function(real t) noexcept { return t; }

struct time_func
{
    output_port y[1];
    time        sigma;

    real default_sigma      = to_real(0.01);
    real (*default_f)(real) = &time_function;

    real value;
    real (*f)(real) = nullptr;

    time_func() noexcept = default;

    time_func(const time_func& other) noexcept
      : sigma(other.sigma)
      , default_sigma(other.default_sigma)
      , default_f(other.default_f)
      , value(other.value)
      , f(other.f)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        f     = default_f;
        sigma = default_sigma;
        value = 0.0;
        return status::success;
    }

    status transition(simulation& /*sim*/,
                      time t,
                      time /*e*/,
                      time /*r*/) noexcept
    {
        value = (*f)(t);
        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        return send_message(sim, y[0], value);
    }

    observation_message observation(const time /*t*/) const noexcept
    {
        return { value };
    }
};

using adder_2 = adder<2>;
using adder_3 = adder<3>;
using adder_4 = adder<4>;

using mult_2 = mult<2>;
using mult_3 = mult<3>;
using mult_4 = mult<4>;

using accumulator_2 = accumulator<2>;

struct queue
{
    input_port  x[1];
    output_port y[1];
    time        sigma;

    u64 fifo = static_cast<u64>(-1);

    real default_ta = one;

    queue() noexcept = default;

    queue(const queue& other) noexcept
      : sigma(other.sigma)
      , fifo(static_cast<u64>(-1))
      , default_ta(other.default_ta)
    {}

    status initialize(simulation& /*sim*/) noexcept
    {
        if (default_ta <= 0)
            irt_bad_return(status::model_queue_bad_ta);

        sigma = time_domain<time>::infinity;
        fifo  = static_cast<u64>(-1);

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        append_dated_message(sim, fifo).clear();
        return status::success;
    }

    status transition(simulation& sim, time t, time /*e*/, time /*r*/) noexcept
    {
        auto list = append_dated_message(sim, fifo);
        while (!list.empty() && list.front().data[0] <= t)
            list.pop_front();

        auto span = get_message(sim, x[0]);
        for (const auto& msg : span) {
            if (!can_alloc_dated_message(sim, 1))
                return status::model_queue_full;

            list.emplace_back(
              irt::real(t + default_ta), msg[0], msg[1], msg[2]);
        }

        if (!list.empty()) {
            sigma = list.front()[0] - t;
            sigma = sigma <= time_domain<time>::zero ? time_domain<time>::zero
                                                     : sigma;
        } else {
            sigma = time_domain<time>::infinity;
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if (fifo == static_cast<u64>(-1))
            return status::success;

        auto       list = append_dated_message(sim, fifo);
        auto       it   = list.begin();
        auto       end  = list.end();
        const auto t    = it->data[0];

        for (; it != end && it->data[0] <= t; ++it)
            irt_return_if_bad(
              send_message(sim, y[0], it->data[1], it->data[2], it->data[3]));

        return status::success;
    }
};

struct dynamic_queue
{
    input_port  x[1];
    output_port y[1];
    time        sigma;
    u64         fifo = static_cast<u64>(-1);

    source default_source_ta;
    bool   stop_on_error = false;

    dynamic_queue() noexcept = default;

    dynamic_queue(const dynamic_queue& other) noexcept
      : sigma(other.sigma)
      , fifo(static_cast<u64>(-1))
      , default_source_ta(other.default_source_ta)
      , stop_on_error(other.stop_on_error)
    {}

    status initialize(simulation& sim) noexcept
    {
        sigma = time_domain<time>::infinity;
        fifo  = static_cast<u64>(-1);

        if (stop_on_error)
            irt_return_if_bad(initialize_source(sim, default_source_ta));
        else
            (void)initialize_source(sim, default_source_ta);

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        append_dated_message(sim, fifo).clear();
        irt_return_if_bad(finalize_source(sim, default_source_ta));

        return status::success;
    }

    status transition(simulation& sim, time t, time /*e*/, time /*r*/) noexcept
    {
        auto list = append_dated_message(sim, fifo);
        while (!list.empty() && list.front().data[0] <= t)
            list.pop_front();

        auto span = get_message(sim, x[0]);
        for (const auto& msg : span) {
            if (!can_alloc_dated_message(sim, 1))
                return status::model_dynamic_queue_full;

            double ta;
            if (stop_on_error) {
                irt_return_if_bad(update_source(sim, default_source_ta, ta));
                list.emplace_back(
                  t + static_cast<real>(ta), msg[0], msg[1], msg[2]);
            } else {
                if (is_success(update_source(sim, default_source_ta, ta)))
                    list.emplace_back(
                      t + static_cast<real>(ta), msg[0], msg[1], msg[2]);
            }
        }

        if (!list.empty()) {
            sigma = list.front().data[0] - t;
            sigma = sigma <= time_domain<time>::zero ? time_domain<time>::zero
                                                     : sigma;
        } else {
            sigma = time_domain<time>::infinity;
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if (fifo == static_cast<u64>(-1))
            return status::success;

        auto       list = append_dated_message(sim, fifo);
        auto       it   = list.begin();
        auto       end  = list.end();
        const auto t    = it->data[0];

        for (; it != end && it->data[0] <= t; ++it)
            irt_return_if_bad(
              send_message(sim, y[0], it->data[1], it->data[2], it->data[3]));

        return status::success;
    }
};

struct priority_queue
{
    input_port  x[1];
    output_port y[1];
    time        sigma;
    u64         fifo       = static_cast<u64>(-1);
    real        default_ta = 1.0;

    source default_source_ta;
    bool   stop_on_error = false;

    priority_queue() noexcept = default;

    priority_queue(const priority_queue& other) noexcept
      : sigma(other.sigma)
      , fifo(static_cast<u64>(-1))
      , default_ta(other.default_ta)
      , default_source_ta(other.default_source_ta)
      , stop_on_error(other.stop_on_error)
    {}

private:
    status try_to_insert(simulation&    sim,
                         const time     t,
                         const message& msg) noexcept
    {
        if (!can_alloc_dated_message(sim, 1))
            irt_bad_return(status::model_priority_queue_source_is_null);

        auto list = append_dated_message(sim, fifo);
        if (list.empty() || list.begin()->data[0] > t) {
            list.emplace_front(irt::real(t), msg[0], msg[1], msg[2]);
        } else {
            auto it  = list.begin();
            auto end = list.end();
            ++it;

            for (; it != end; ++it) {
                if (it->data[0] > t) {
                    list.emplace(it, irt::real(t), msg[0], msg[1], msg[2]);
                    return status::success;
                }
            }
        }

        return status::success;
    }

public:
    status initialize(simulation& sim) noexcept
    {
        if (stop_on_error)
            irt_return_if_bad(initialize_source(sim, default_source_ta));
        else
            (void)initialize_source(sim, default_source_ta);

        sigma = time_domain<time>::infinity;
        fifo  = static_cast<u64>(-1);

        return status::success;
    }

    status finalize(simulation& sim) noexcept
    {
        append_dated_message(sim, fifo).clear();
        irt_return_if_bad(finalize_source(sim, default_source_ta));

        return status::success;
    }

    status transition(simulation& sim, time t, time /*e*/, time /*r*/) noexcept
    {
        auto list = append_dated_message(sim, fifo);
        while (!list.empty() && list.front().data[0] <= t)
            list.pop_front();

        auto span = get_message(sim, x[0]);
        for (const auto& msg : span) {
            double value;

            if (stop_on_error) {
                irt_return_if_bad(update_source(sim, default_source_ta, value));

                if (auto ret =
                      try_to_insert(sim, static_cast<real>(value) + t, msg);
                    is_bad(ret))
                    irt_bad_return(status::model_priority_queue_full);
            } else {
                if (is_success(update_source(sim, default_source_ta, value))) {
                    if (auto ret =
                          try_to_insert(sim, static_cast<real>(value) + t, msg);
                        is_bad(ret))
                        irt_bad_return(status::model_priority_queue_full);
                }
            }
        }

        if (!list.empty()) {
            sigma = list.front()[0] - t;
            sigma = sigma <= time_domain<time>::zero ? time_domain<time>::zero
                                                     : sigma;
        } else {
            sigma = time_domain<time>::infinity;
        }

        return status::success;
    }

    status lambda(simulation& sim) noexcept
    {
        if (fifo == static_cast<u64>(-1))
            return status::success;

        auto       list = get_dated_message(sim, fifo);
        auto       it   = list.begin();
        auto       end  = list.end();
        const auto t    = it->data[0];

        for (; it != end && it->data[0] <= t; ++it)
            irt_return_if_bad(
              send_message(sim, y[0], it->data[1], it->data[2], it->data[3]));

        return status::success;
    }
};

constexpr sz max(sz a) { return a; }

template<typename... Args>
constexpr sz max(sz a, Args... args)
{
    return std::max(max(args...), a);
}

constexpr sz max_size_in_bytes() noexcept
{
    return max(sizeof(qss1_integrator),
               sizeof(qss1_multiplier),
               sizeof(qss1_cross),
               sizeof(qss1_power),
               sizeof(qss1_square),
               sizeof(qss1_sum_2),
               sizeof(qss1_sum_3),
               sizeof(qss1_sum_4),
               sizeof(qss1_wsum_2),
               sizeof(qss1_wsum_3),
               sizeof(qss1_wsum_4),
               sizeof(qss2_integrator),
               sizeof(qss2_multiplier),
               sizeof(qss2_cross),
               sizeof(qss2_power),
               sizeof(qss2_square),
               sizeof(qss2_sum_2),
               sizeof(qss2_sum_3),
               sizeof(qss2_sum_4),
               sizeof(qss2_wsum_2),
               sizeof(qss2_wsum_3),
               sizeof(qss2_wsum_4),
               sizeof(qss3_integrator),
               sizeof(qss3_multiplier),
               sizeof(qss3_cross),
               sizeof(qss3_power),
               sizeof(qss3_square),
               sizeof(qss3_sum_2),
               sizeof(qss3_sum_3),
               sizeof(qss3_sum_4),
               sizeof(qss3_wsum_2),
               sizeof(qss3_wsum_3),
               sizeof(qss3_wsum_4),
               sizeof(integrator),
               sizeof(quantifier),
               sizeof(adder_2),
               sizeof(adder_3),
               sizeof(adder_4),
               sizeof(mult_2),
               sizeof(mult_3),
               sizeof(mult_4),
               sizeof(counter),
               sizeof(queue),
               sizeof(dynamic_queue),
               sizeof(priority_queue),
               sizeof(generator),
               sizeof(constant),
               sizeof(cross),
               sizeof(time_func),
               sizeof(accumulator_2),
               sizeof(filter),
               sizeof(flow));
}

struct model
{
    real         tl     = 0.0;
    real         tn     = time_domain<time>::infinity;
    heap::handle handle = nullptr;

    observer_id   obs_id = observer_id{ 0 };
    dynamics_type type;

    std::byte dyn[max_size_in_bytes()];
};

template<typename Function, typename... Args>
constexpr auto dispatch(const model& mdl, Function&& f, Args... args) noexcept
{
    switch (mdl.type) {
    case dynamics_type::qss1_integrator:
        return f(*reinterpret_cast<const qss1_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss1_multiplier:
        return f(*reinterpret_cast<const qss1_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss1_cross:
        return f(*reinterpret_cast<const qss1_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss1_power:
        return f(*reinterpret_cast<const qss1_power*>(&mdl.dyn), args...);
    case dynamics_type::qss1_square:
        return f(*reinterpret_cast<const qss1_square*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_2:
        return f(*reinterpret_cast<const qss1_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_3:
        return f(*reinterpret_cast<const qss1_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_4:
        return f(*reinterpret_cast<const qss1_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_2:
        return f(*reinterpret_cast<const qss1_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_3:
        return f(*reinterpret_cast<const qss1_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_4:
        return f(*reinterpret_cast<const qss1_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::qss2_integrator:
        return f(*reinterpret_cast<const qss2_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss2_multiplier:
        return f(*reinterpret_cast<const qss2_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss2_cross:
        return f(*reinterpret_cast<const qss2_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss2_power:
        return f(*reinterpret_cast<const qss2_power*>(&mdl.dyn), args...);
    case dynamics_type::qss2_square:
        return f(*reinterpret_cast<const qss2_square*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_2:
        return f(*reinterpret_cast<const qss2_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_3:
        return f(*reinterpret_cast<const qss2_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_4:
        return f(*reinterpret_cast<const qss2_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_2:
        return f(*reinterpret_cast<const qss2_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_3:
        return f(*reinterpret_cast<const qss2_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_4:
        return f(*reinterpret_cast<const qss2_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::qss3_integrator:
        return f(*reinterpret_cast<const qss3_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss3_multiplier:
        return f(*reinterpret_cast<const qss3_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss3_cross:
        return f(*reinterpret_cast<const qss3_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss3_power:
        return f(*reinterpret_cast<const qss3_power*>(&mdl.dyn), args...);
    case dynamics_type::qss3_square:
        return f(*reinterpret_cast<const qss3_square*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_2:
        return f(*reinterpret_cast<const qss3_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_3:
        return f(*reinterpret_cast<const qss3_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_4:
        return f(*reinterpret_cast<const qss3_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_2:
        return f(*reinterpret_cast<const qss3_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_3:
        return f(*reinterpret_cast<const qss3_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_4:
        return f(*reinterpret_cast<const qss3_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::integrator:
        return f(*reinterpret_cast<const integrator*>(&mdl.dyn), args...);
    case dynamics_type::quantifier:
        return f(*reinterpret_cast<const quantifier*>(&mdl.dyn), args...);
    case dynamics_type::adder_2:
        return f(*reinterpret_cast<const adder_2*>(&mdl.dyn), args...);
    case dynamics_type::adder_3:
        return f(*reinterpret_cast<const adder_3*>(&mdl.dyn), args...);
    case dynamics_type::adder_4:
        return f(*reinterpret_cast<const adder_4*>(&mdl.dyn), args...);
    case dynamics_type::mult_2:
        return f(*reinterpret_cast<const mult_2*>(&mdl.dyn), args...);
    case dynamics_type::mult_3:
        return f(*reinterpret_cast<const mult_3*>(&mdl.dyn), args...);
    case dynamics_type::mult_4:
        return f(*reinterpret_cast<const mult_4*>(&mdl.dyn), args...);
    case dynamics_type::counter:
        return f(*reinterpret_cast<const counter*>(&mdl.dyn), args...);
    case dynamics_type::queue:
        return f(*reinterpret_cast<const queue*>(&mdl.dyn), args...);
    case dynamics_type::dynamic_queue:
        return f(*reinterpret_cast<const dynamic_queue*>(&mdl.dyn), args...);
    case dynamics_type::priority_queue:
        return f(*reinterpret_cast<const priority_queue*>(&mdl.dyn), args...);
    case dynamics_type::generator:
        return f(*reinterpret_cast<const generator*>(&mdl.dyn), args...);
    case dynamics_type::constant:
        return f(*reinterpret_cast<const constant*>(&mdl.dyn), args...);
    case dynamics_type::cross:
        return f(*reinterpret_cast<const cross*>(&mdl.dyn), args...);
    case dynamics_type::accumulator_2:
        return f(*reinterpret_cast<const accumulator_2*>(&mdl.dyn), args...);
    case dynamics_type::time_func:
        return f(*reinterpret_cast<const time_func*>(&mdl.dyn), args...);
    case dynamics_type::filter:
        return f(*reinterpret_cast<const filter*>(&mdl.dyn), args...);
    case dynamics_type::flow:
        return f(*reinterpret_cast<const flow*>(&mdl.dyn), args...);
    }

    irt_unreachable();
}

template<typename Function, typename... Args>
constexpr auto dispatch(model& mdl, Function&& f, Args... args) noexcept
{
    switch (mdl.type) {
    case dynamics_type::qss1_integrator:
        return f(*reinterpret_cast<qss1_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss1_multiplier:
        return f(*reinterpret_cast<qss1_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss1_cross:
        return f(*reinterpret_cast<qss1_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss1_power:
        return f(*reinterpret_cast<qss1_power*>(&mdl.dyn), args...);
    case dynamics_type::qss1_square:
        return f(*reinterpret_cast<qss1_square*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_2:
        return f(*reinterpret_cast<qss1_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_3:
        return f(*reinterpret_cast<qss1_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss1_sum_4:
        return f(*reinterpret_cast<qss1_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_2:
        return f(*reinterpret_cast<qss1_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_3:
        return f(*reinterpret_cast<qss1_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss1_wsum_4:
        return f(*reinterpret_cast<qss1_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::qss2_integrator:
        return f(*reinterpret_cast<qss2_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss2_multiplier:
        return f(*reinterpret_cast<qss2_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss2_cross:
        return f(*reinterpret_cast<qss2_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss2_power:
        return f(*reinterpret_cast<qss2_power*>(&mdl.dyn), args...);
    case dynamics_type::qss2_square:
        return f(*reinterpret_cast<qss2_square*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_2:
        return f(*reinterpret_cast<qss2_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_3:
        return f(*reinterpret_cast<qss2_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss2_sum_4:
        return f(*reinterpret_cast<qss2_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_2:
        return f(*reinterpret_cast<qss2_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_3:
        return f(*reinterpret_cast<qss2_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss2_wsum_4:
        return f(*reinterpret_cast<qss2_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::qss3_integrator:
        return f(*reinterpret_cast<qss3_integrator*>(&mdl.dyn), args...);
    case dynamics_type::qss3_multiplier:
        return f(*reinterpret_cast<qss3_multiplier*>(&mdl.dyn), args...);
    case dynamics_type::qss3_cross:
        return f(*reinterpret_cast<qss3_cross*>(&mdl.dyn), args...);
    case dynamics_type::qss3_power:
        return f(*reinterpret_cast<qss3_power*>(&mdl.dyn), args...);
    case dynamics_type::qss3_square:
        return f(*reinterpret_cast<qss3_square*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_2:
        return f(*reinterpret_cast<qss3_sum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_3:
        return f(*reinterpret_cast<qss3_sum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss3_sum_4:
        return f(*reinterpret_cast<qss3_sum_4*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_2:
        return f(*reinterpret_cast<qss3_wsum_2*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_3:
        return f(*reinterpret_cast<qss3_wsum_3*>(&mdl.dyn), args...);
    case dynamics_type::qss3_wsum_4:
        return f(*reinterpret_cast<qss3_wsum_4*>(&mdl.dyn), args...);

    case dynamics_type::integrator:
        return f(*reinterpret_cast<integrator*>(&mdl.dyn), args...);
    case dynamics_type::quantifier:
        return f(*reinterpret_cast<quantifier*>(&mdl.dyn), args...);
    case dynamics_type::adder_2:
        return f(*reinterpret_cast<adder_2*>(&mdl.dyn), args...);
    case dynamics_type::adder_3:
        return f(*reinterpret_cast<adder_3*>(&mdl.dyn), args...);
    case dynamics_type::adder_4:
        return f(*reinterpret_cast<adder_4*>(&mdl.dyn), args...);
    case dynamics_type::mult_2:
        return f(*reinterpret_cast<mult_2*>(&mdl.dyn), args...);
    case dynamics_type::mult_3:
        return f(*reinterpret_cast<mult_3*>(&mdl.dyn), args...);
    case dynamics_type::mult_4:
        return f(*reinterpret_cast<mult_4*>(&mdl.dyn), args...);
    case dynamics_type::counter:
        return f(*reinterpret_cast<counter*>(&mdl.dyn), args...);
    case dynamics_type::queue:
        return f(*reinterpret_cast<queue*>(&mdl.dyn), args...);
    case dynamics_type::dynamic_queue:
        return f(*reinterpret_cast<dynamic_queue*>(&mdl.dyn), args...);
    case dynamics_type::priority_queue:
        return f(*reinterpret_cast<priority_queue*>(&mdl.dyn), args...);
    case dynamics_type::generator:
        return f(*reinterpret_cast<generator*>(&mdl.dyn), args...);
    case dynamics_type::constant:
        return f(*reinterpret_cast<constant*>(&mdl.dyn), args...);
    case dynamics_type::cross:
        return f(*reinterpret_cast<cross*>(&mdl.dyn), args...);
    case dynamics_type::accumulator_2:
        return f(*reinterpret_cast<accumulator_2*>(&mdl.dyn), args...);
    case dynamics_type::time_func:
        return f(*reinterpret_cast<time_func*>(&mdl.dyn), args...);
    case dynamics_type::filter:
        return f(*reinterpret_cast<filter*>(&mdl.dyn), args...);
    case dynamics_type::flow:
        return f(*reinterpret_cast<flow*>(&mdl.dyn), args...);
    }

    irt_unreachable();
}

inline status get_input_port(model& src, int port_src, input_port*& p) noexcept
{
    return dispatch(
      src, [port_src, &p]<typename Dynamics>(Dynamics& dyn) -> status {
          if constexpr (is_detected_v<has_input_port_t, Dynamics>) {
              if (port_src >= 0 && port_src < length(dyn.x)) {
                  p = &dyn.x[port_src];
                  return status::success;
              }
          }

          return status::model_connect_output_port_unknown;
      });
}

inline status get_output_port(model&        dst,
                              int           port_dst,
                              output_port*& p) noexcept
{
    return dispatch(
      dst, [port_dst, &p]<typename Dynamics>(Dynamics& dyn) -> status {
          if constexpr (is_detected_v<has_output_port_t, Dynamics>) {
              if (port_dst >= 0 && port_dst < length(dyn.y)) {
                  p = &dyn.y[port_dst];
                  return status::success;
              }
          }

          return status::model_connect_output_port_unknown;
      });
}

inline bool is_ports_compatible(const model&               mdl_src,
                                [[maybe_unused]] const int o_port_index,
                                const model&               mdl_dst,
                                const int i_port_index) noexcept
{
    if (&mdl_src == &mdl_dst)
        return false;

    switch (mdl_src.type) {
    case dynamics_type::quantifier:
        if (mdl_dst.type == dynamics_type::integrator &&
            i_port_index ==
              static_cast<int>(integrator::port_name::port_quanta))
            return true;

        return false;

    case dynamics_type::qss1_integrator:
    case dynamics_type::qss1_multiplier:
    case dynamics_type::qss1_cross:
    case dynamics_type::qss1_power:
    case dynamics_type::qss1_square:
    case dynamics_type::qss1_sum_2:
    case dynamics_type::qss1_sum_3:
    case dynamics_type::qss1_sum_4:
    case dynamics_type::qss1_wsum_2:
    case dynamics_type::qss1_wsum_3:
    case dynamics_type::qss1_wsum_4:
    case dynamics_type::qss2_integrator:
    case dynamics_type::qss2_multiplier:
    case dynamics_type::qss2_cross:
    case dynamics_type::qss2_power:
    case dynamics_type::qss2_square:
    case dynamics_type::qss2_sum_2:
    case dynamics_type::qss2_sum_3:
    case dynamics_type::qss2_sum_4:
    case dynamics_type::qss2_wsum_2:
    case dynamics_type::qss2_wsum_3:
    case dynamics_type::qss2_wsum_4:
    case dynamics_type::qss3_integrator:
    case dynamics_type::qss3_multiplier:
    case dynamics_type::qss3_cross:
    case dynamics_type::qss3_power:
    case dynamics_type::qss3_square:
    case dynamics_type::qss3_sum_2:
    case dynamics_type::qss3_sum_3:
    case dynamics_type::qss3_sum_4:
    case dynamics_type::qss3_wsum_2:
    case dynamics_type::qss3_wsum_3:
    case dynamics_type::qss3_wsum_4:
    case dynamics_type::integrator:
    case dynamics_type::adder_2:
    case dynamics_type::adder_3:
    case dynamics_type::adder_4:
    case dynamics_type::mult_2:
    case dynamics_type::mult_3:
    case dynamics_type::mult_4:
    case dynamics_type::counter:
    case dynamics_type::queue:
    case dynamics_type::dynamic_queue:
    case dynamics_type::priority_queue:
    case dynamics_type::generator:
    case dynamics_type::constant:
    case dynamics_type::cross:
    case dynamics_type::time_func:
    case dynamics_type::filter:
    case dynamics_type::flow:
    case dynamics_type::accumulator_2:
        if (mdl_dst.type == dynamics_type::integrator &&
            i_port_index ==
              static_cast<int>(integrator::port_name::port_quanta))
            return false;

        return true;
    }

    return false;
}

inline status global_connect(simulation& sim,
                             model&      src,
                             int         port_src,
                             model_id    dst,
                             int         port_dst) noexcept
{
    return dispatch(
      src,
      [&sim, port_src, dst, port_dst]<typename Dynamics>(
        Dynamics& dyn) -> status {
          if constexpr (is_detected_v<has_output_port_t, Dynamics>) {
              auto list = append_node(sim, dyn.y[static_cast<u8>(port_src)]);
              for (const auto& elem : list) {
                  irt_return_if_fail(
                    !(elem.model == dst && elem.port_index == port_dst),
                    status::model_connect_already_exist);
              }

              irt_return_if_fail(can_alloc_node(sim, 1),
                                 status::simulation_not_enough_connection);

              list.emplace_back(dst, static_cast<i8>(port_dst));

              return status::success;
          }

          irt_unreachable();
      });
}

inline status global_disconnect(simulation& sim,
                                model&      src,
                                int         port_src,
                                model_id    dst,
                                int         port_dst) noexcept
{
    return dispatch(
      src,
      [&sim, port_src, dst, port_dst]<typename Dynamics>(
        Dynamics& dyn) -> status {
          if constexpr (is_detected_v<has_output_port_t, Dynamics>) {
              auto list = append_node(sim, dyn.y[port_src]);
              for (auto it = list.begin(), end = list.end(); it != end; ++it) {
                  if (it->model == dst && it->port_index == port_dst) {
                      it = list.erase(it);
                      return status::success;
                  }
              }
          }

          irt_unreachable();
      });
}

/*****************************************************************************
 *
 * scheduller
 *
 ****************************************************************************/

class scheduller
{
private:
    heap m_heap;

public:
    scheduller() = default;

    status init(size_t capacity) noexcept
    {
        irt_return_if_bad(m_heap.init(capacity));

        return status::success;
    }

    void clear() { m_heap.clear(); }

    //! @brief Insert a newly model into the scheduller.
    void insert(model& mdl, model_id id, time tn) noexcept
    {
        irt_assert(mdl.handle == nullptr);

        mdl.handle = m_heap.insert(tn, id);
    }

    //! @brief Reintegrate or reinsert an old popped model into the
    //! scheduller.
    void reintegrate(model& mdl, time tn) noexcept
    {
        irt_assert(mdl.handle != nullptr);

        mdl.handle->tn = tn;

        m_heap.insert(mdl.handle);
    }

    void erase(model& mdl) noexcept
    {
        if (mdl.handle) {
            m_heap.remove(mdl.handle);
            m_heap.destroy(mdl.handle);
            mdl.handle = nullptr;
        }
    }

    void update(model& mdl, time tn) noexcept
    {
        irt_assert(mdl.handle != nullptr);

        mdl.handle->tn = tn;

        irt_assert(tn <= mdl.tn);

        if (tn < mdl.tn)
            m_heap.decrease(mdl.handle);
        else if (tn > mdl.tn)
            m_heap.increase(mdl.handle);
    }

    void pop(vector<model_id>& out) noexcept
    {
        time t = tn();

        out.clear();
        out.emplace_back(m_heap.pop()->id);

        while (!m_heap.empty() && t == tn())
            out.emplace_back(m_heap.pop()->id);
    }

    time tn() const noexcept { return m_heap.top()->tn; }

    bool empty() const noexcept { return m_heap.empty(); }

    size_t size() const noexcept { return m_heap.size(); }
};

/*****************************************************************************
 *
 * simulation
 *
 ****************************************************************************/

template<typename Dynamics>
static constexpr dynamics_type dynamics_typeof() noexcept
{
    if constexpr (std::is_same_v<Dynamics, qss1_integrator>)
        return dynamics_type::qss1_integrator;
    if constexpr (std::is_same_v<Dynamics, qss1_multiplier>)
        return dynamics_type::qss1_multiplier;
    if constexpr (std::is_same_v<Dynamics, qss1_cross>)
        return dynamics_type::qss1_cross;
    if constexpr (std::is_same_v<Dynamics, qss1_power>)
        return dynamics_type::qss1_power;
    if constexpr (std::is_same_v<Dynamics, qss1_square>)
        return dynamics_type::qss1_square;
    if constexpr (std::is_same_v<Dynamics, qss1_sum_2>)
        return dynamics_type::qss1_sum_2;
    if constexpr (std::is_same_v<Dynamics, qss1_sum_3>)
        return dynamics_type::qss1_sum_3;
    if constexpr (std::is_same_v<Dynamics, qss1_sum_4>)
        return dynamics_type::qss1_sum_4;
    if constexpr (std::is_same_v<Dynamics, qss1_wsum_2>)
        return dynamics_type::qss1_wsum_2;
    if constexpr (std::is_same_v<Dynamics, qss1_wsum_3>)
        return dynamics_type::qss1_wsum_3;
    if constexpr (std::is_same_v<Dynamics, qss1_wsum_4>)
        return dynamics_type::qss1_wsum_4;

    if constexpr (std::is_same_v<Dynamics, qss2_integrator>)
        return dynamics_type::qss2_integrator;
    if constexpr (std::is_same_v<Dynamics, qss2_multiplier>)
        return dynamics_type::qss2_multiplier;
    if constexpr (std::is_same_v<Dynamics, qss2_cross>)
        return dynamics_type::qss2_cross;
    if constexpr (std::is_same_v<Dynamics, qss2_power>)
        return dynamics_type::qss2_power;
    if constexpr (std::is_same_v<Dynamics, qss2_square>)
        return dynamics_type::qss2_square;
    if constexpr (std::is_same_v<Dynamics, qss2_sum_2>)
        return dynamics_type::qss2_sum_2;
    if constexpr (std::is_same_v<Dynamics, qss2_sum_3>)
        return dynamics_type::qss2_sum_3;
    if constexpr (std::is_same_v<Dynamics, qss2_sum_4>)
        return dynamics_type::qss2_sum_4;
    if constexpr (std::is_same_v<Dynamics, qss2_wsum_2>)
        return dynamics_type::qss2_wsum_2;
    if constexpr (std::is_same_v<Dynamics, qss2_wsum_3>)
        return dynamics_type::qss2_wsum_3;
    if constexpr (std::is_same_v<Dynamics, qss2_wsum_4>)
        return dynamics_type::qss2_wsum_4;

    if constexpr (std::is_same_v<Dynamics, qss3_integrator>)
        return dynamics_type::qss3_integrator;
    if constexpr (std::is_same_v<Dynamics, qss3_multiplier>)
        return dynamics_type::qss3_multiplier;
    if constexpr (std::is_same_v<Dynamics, qss3_cross>)
        return dynamics_type::qss3_cross;
    if constexpr (std::is_same_v<Dynamics, qss3_power>)
        return dynamics_type::qss3_power;
    if constexpr (std::is_same_v<Dynamics, qss3_square>)
        return dynamics_type::qss3_square;
    if constexpr (std::is_same_v<Dynamics, qss3_sum_2>)
        return dynamics_type::qss3_sum_2;
    if constexpr (std::is_same_v<Dynamics, qss3_sum_3>)
        return dynamics_type::qss3_sum_3;
    if constexpr (std::is_same_v<Dynamics, qss3_sum_4>)
        return dynamics_type::qss3_sum_4;
    if constexpr (std::is_same_v<Dynamics, qss3_wsum_2>)
        return dynamics_type::qss3_wsum_2;
    if constexpr (std::is_same_v<Dynamics, qss3_wsum_3>)
        return dynamics_type::qss3_wsum_3;
    if constexpr (std::is_same_v<Dynamics, qss3_wsum_4>)
        return dynamics_type::qss3_wsum_4;
    if constexpr (std::is_same_v<Dynamics, integrator>)
        return dynamics_type::integrator;
    if constexpr (std::is_same_v<Dynamics, quantifier>)
        return dynamics_type::quantifier;
    if constexpr (std::is_same_v<Dynamics, adder_2>)
        return dynamics_type::adder_2;
    if constexpr (std::is_same_v<Dynamics, adder_3>)
        return dynamics_type::adder_3;
    if constexpr (std::is_same_v<Dynamics, adder_4>)
        return dynamics_type::adder_4;
    if constexpr (std::is_same_v<Dynamics, mult_2>)
        return dynamics_type::mult_2;
    if constexpr (std::is_same_v<Dynamics, mult_3>)
        return dynamics_type::mult_3;
    if constexpr (std::is_same_v<Dynamics, mult_4>)
        return dynamics_type::mult_4;
    if constexpr (std::is_same_v<Dynamics, counter>)
        return dynamics_type::counter;
    if constexpr (std::is_same_v<Dynamics, queue>)
        return dynamics_type::queue;
    if constexpr (std::is_same_v<Dynamics, dynamic_queue>)
        return dynamics_type::dynamic_queue;
    if constexpr (std::is_same_v<Dynamics, priority_queue>)
        return dynamics_type::priority_queue;
    if constexpr (std::is_same_v<Dynamics, generator>)
        return dynamics_type::generator;
    if constexpr (std::is_same_v<Dynamics, constant>)
        return dynamics_type::constant;
    if constexpr (std::is_same_v<Dynamics, cross>)
        return dynamics_type::cross;
    if constexpr (std::is_same_v<Dynamics, time_func>)
        return dynamics_type::time_func;
    if constexpr (std::is_same_v<Dynamics, accumulator_2>)
        return dynamics_type::accumulator_2;
    if constexpr (std::is_same_v<Dynamics, filter>)
        return dynamics_type::filter;
    if constexpr (std::is_same_v<Dynamics, flow>)
        return dynamics_type::flow;

    irt_unreachable();
}

template<typename Dynamics>
Dynamics& get_dyn(model& mdl) noexcept
{
    irt_assert(dynamics_typeof<Dynamics>() == mdl.type);
    return *reinterpret_cast<Dynamics*>(&mdl.dyn);
}

template<typename Dynamics>
const Dynamics& get_dyn(const model& mdl) noexcept
{
    irt_assert(dynamics_typeof<Dynamics>() == mdl.type);
    return *reinterpret_cast<const Dynamics*>(&mdl.dyn);
}

template<typename Dynamics>
constexpr const model& get_model(const Dynamics& d) noexcept
{
    const Dynamics* __mptr = &d;
    return *(const model*)((const char*)__mptr - offsetof(model, dyn));
}

template<typename Dynamics>
constexpr model& get_model(Dynamics& d) noexcept
{
    Dynamics* __mptr = &d;
    return *(model*)((char*)__mptr - offsetof(model, dyn));
}

struct simulation
{
    block_allocator<list_view_node<message>>       message_alloc;
    block_allocator<list_view_node<node>>          node_alloc;
    block_allocator<list_view_node<record>>        record_alloc;
    block_allocator<list_view_node<dated_message>> dated_message_alloc;
    vector<output_message>                         emitting_output_ports;
    vector<model_id>                               immediate_models;
    data_array<model, model_id>                    models;
    data_array<observer, observer_id>              observers;

    scheduller sched;

    //! @brief Use initialize, generate or finalize data from a source.
    //!
    //! See the @c external_source class for an implementation.
    function_ref<status(source&, const source::operation_type)> source_dispatch;

    model_id get_id(const model& mdl) const { return models.get_id(mdl); }

    template<typename Dynamics>
    model_id get_id(const Dynamics& dyn) const
    {
        return models.get_id(get_model(dyn));
    }

public:
    status init(size_t model_capacity, size_t messages_capacity)
    {
        constexpr size_t ten{ 10 };

        irt_return_if_bad(message_alloc.init(messages_capacity));
        irt_return_if_bad(node_alloc.init(model_capacity * ten));
        irt_return_if_bad(record_alloc.init(model_capacity * ten));
        irt_return_if_bad(dated_message_alloc.init(model_capacity));
        irt_return_if_bad(models.init(model_capacity));
        irt_return_if_bad(observers.init(model_capacity));
        irt_return_if_bad(sched.init(model_capacity));

        emitting_output_ports.reserve(static_cast<i32>(model_capacity));
        immediate_models.reserve(static_cast<i32>(model_capacity));

        return status::success;
    }

    bool can_alloc(int place = 1) const noexcept
    {
        return models.can_alloc(place);
    }

    //! @brief cleanup simulation object
    //!
    //! Clean scheduller and input/output port from message.
    void clean() noexcept
    {
        sched.clear();

        message_alloc.reset();
        record_alloc.reset();
        dated_message_alloc.reset();

        emitting_output_ports.clear();
        immediate_models.clear();
    }

    //! @brief cleanup simulation and destroy all models and connections
    void clear() noexcept
    {
        clean();

        node_alloc.reset();

        models.clear();
        observers.clear();
    }

    //! @brief This function allocates dynamics and models.
    template<typename Dynamics>
    Dynamics& alloc() noexcept
    {
        irt_assert(!models.full());

        auto& mdl  = models.alloc();
        mdl.type   = dynamics_typeof<Dynamics>();
        mdl.handle = nullptr;

        new (&mdl.dyn) Dynamics{};
        auto& dyn = get_dyn<Dynamics>(mdl);

        if constexpr (is_detected_v<has_input_port_t, Dynamics>)
            for (int i = 0, e = length(dyn.x); i != e; ++i)
                dyn.x[i] = static_cast<u64>(-1);

        if constexpr (is_detected_v<has_output_port_t, Dynamics>)
            for (int i = 0, e = length(dyn.y); i != e; ++i)
                dyn.y[i] = static_cast<u64>(-1);

        return dyn;
    }

    //! @brief This function allocates dynamics and models.
    model& clone(const model& mdl) noexcept
    {
        /* Use can_alloc before using this function. */
        irt_assert(!models.full());

        auto& new_mdl  = models.alloc();
        new_mdl.type   = mdl.type;
        new_mdl.handle = nullptr;

        dispatch(new_mdl, [&mdl]<typename Dynamics>(Dynamics& dyn) -> void {
            const auto& src_dyn = get_dyn<Dynamics>(mdl);
            new (&dyn) Dynamics(src_dyn);

            if constexpr (is_detected_v<has_input_port_t, Dynamics>)
                for (int i = 0, e = length(dyn.x); i != e; ++i)
                    dyn.x[i] = static_cast<u64>(-1);

            if constexpr (is_detected_v<has_output_port_t, Dynamics>)
                for (int i = 0, e = length(dyn.y); i != e; ++i)
                    dyn.y[i] = static_cast<u64>(-1);
        });

        return new_mdl;
    }

    //! @brief This function allocates dynamics and models.
    model& alloc(dynamics_type type) noexcept
    {
        irt_assert(!models.full());

        auto& mdl  = models.alloc();
        mdl.type   = type;
        mdl.handle = nullptr;

        dispatch(mdl, []<typename Dynamics>(Dynamics& dyn) -> void {
            new (&dyn) Dynamics{};

            if constexpr (is_detected_v<has_input_port_t, Dynamics>)
                for (int i = 0, e = length(dyn.x); i != e; ++i)
                    dyn.x[i] = static_cast<u64>(-1);

            if constexpr (is_detected_v<has_output_port_t, Dynamics>)
                for (int i = 0, e = length(dyn.y); i != e; ++i)
                    dyn.y[i] = static_cast<u64>(-1);
        });

        return mdl;
    }

    void observe(model& mdl, observer& obs) noexcept
    {
        mdl.obs_id = observers.get_id(obs);
        obs.model  = models.get_id(mdl);
    }

    status deallocate(model_id id)
    {
        auto* mdl = models.try_to_get(id);
        irt_return_if_fail(mdl, status::unknown_dynamics);

        if (auto* obs = observers.try_to_get(mdl->obs_id); obs) {
            obs->model  = static_cast<model_id>(0);
            mdl->obs_id = static_cast<observer_id>(0);
            observers.free(*obs);
        }

        dispatch(*mdl, [this]<typename Dynamics>(Dynamics& dyn) {
            this->do_deallocate<Dynamics>(dyn);
        });

        sched.erase(*mdl);
        models.free(*mdl);

        return status::success;
    }

    template<typename Dynamics>
    void do_deallocate(Dynamics& dyn) noexcept
    {
        if constexpr (is_detected_v<has_output_port_t, Dynamics>) {
            for (auto& elem : dyn.y)
                append_node(*this, elem).clear();
        }

        if constexpr (is_detected_v<has_input_port_t, Dynamics>) {
            for (auto& elem : dyn.x)
                append_message(*this, elem).clear();
        }

        dyn.~Dynamics();
    }

    bool can_connect(int number) const noexcept
    {
        return can_alloc_node(*this, number);
    }

    status connect(model& src, int port_src, model& dst, int port_dst) noexcept
    {
        irt_return_if_fail(is_ports_compatible(src, port_src, dst, port_dst),
                           status::model_connect_bad_dynamics);

        const auto dst_id = get_id(dst);

        return global_connect(*this, src, port_src, dst_id, port_dst);
    }

    template<typename DynamicsSrc, typename DynamicsDst>
    status connect(DynamicsSrc& src,
                   int          port_src,
                   DynamicsDst& dst,
                   int          port_dst) noexcept
    {
        model&   src_model    = get_model(src);
        model&   dst_model    = get_model(dst);
        model_id model_dst_id = get_id(dst);

        irt_return_if_fail(
          is_ports_compatible(src_model, port_src, dst_model, port_dst),
          status::model_connect_bad_dynamics);

        return global_connect(
          *this, src_model, port_src, model_dst_id, port_dst);
    }

    status disconnect(model& src,
                      int    port_src,
                      model& dst,
                      int    port_dst) noexcept
    {
        return global_disconnect(*this, src, port_src, get_id(dst), port_dst);
    }

    status initialize(time t) noexcept
    {
        clean();

        irt::model* mdl = nullptr;
        while (models.next(mdl))
            irt_return_if_bad(make_initialize(*mdl, t));

        irt::observer* obs = nullptr;
        while (observers.next(obs)) {
            if (auto* mdl = models.try_to_get(obs->model); mdl) {
                obs->msg.reset();
                obs->cb(
                  *obs, mdl->type, mdl->tl, t, observer::status::initialize);
            }
        }

        return status::success;
    }

    status run(time& t) noexcept
    {
        if (sched.empty()) {
            t = time_domain<time>::infinity;
            return status::success;
        }

        if (t = sched.tn(); time_domain<time>::is_infinity(t))
            return status::success;

        immediate_models.clear();
        sched.pop(immediate_models);

        emitting_output_ports.clear();
        for (const auto id : immediate_models)
            if (auto* mdl = models.try_to_get(id); mdl)
                irt_return_if_bad(make_transition(*mdl, t));

        for (int i = 0, e = length(emitting_output_ports); i != e; ++i) {
            auto* mdl = models.try_to_get(emitting_output_ports[i].model);
            if (!mdl)
                continue;

            sched.update(*mdl, t);

            irt_return_if_fail(can_alloc_message(*this, 1),
                               status::simulation_not_enough_message);

            auto  port = emitting_output_ports[i].port;
            auto& msg  = emitting_output_ports[i].msg;

            dispatch(
              *mdl, [this, port, &msg]<typename Dynamics>(Dynamics& dyn) {
                  if constexpr (is_detected_v<has_input_port_t, Dynamics>) {
                      auto list = append_message(*this, dyn.x[port]);
                      list.push_back(msg);
                  }
              });
        }

        return status::success;
    }

    template<typename Dynamics>
    status make_initialize(model& mdl, Dynamics& dyn, time t) noexcept
    {
        if constexpr (is_detected_v<has_input_port_t, Dynamics>) {
            for (int i = 0, e = length(dyn.x); i != e; ++i)
                dyn.x[i] = static_cast<u64>(-1);
        }

        if constexpr (is_detected_v<initialize_function_t, Dynamics>)
            irt_return_if_bad(dyn.initialize(*this));

        mdl.tl     = t;
        mdl.tn     = t + dyn.sigma;
        mdl.handle = nullptr;

        sched.insert(mdl, models.get_id(mdl), mdl.tn);

        return status::success;
    }

    status make_initialize(model& mdl, time t) noexcept
    {
        return dispatch(mdl, [this, &mdl, t]<typename Dynamics>(Dynamics& dyn) {
            return this->make_initialize(mdl, dyn, t);
        });
    }

    template<typename Dynamics>
    status make_transition(model& mdl, Dynamics& dyn, time t) noexcept
    {
        if constexpr (is_detected_v<observation_function_t, Dynamics>) {
            if (mdl.obs_id != static_cast<observer_id>(0)) {
                if (auto* obs = observers.try_to_get(mdl.obs_id); obs) {
                    obs->msg = dyn.observation(t - mdl.tl);
                    obs->cb(*obs, mdl.type, mdl.tl, t, observer::status::run);
                } else {
                    mdl.obs_id = static_cast<observer_id>(0);
                }
            }
        }

        if (mdl.tn == mdl.handle->tn) {
            if constexpr (is_detected_v<lambda_function_t, Dynamics>)
                if constexpr (is_detected_v<has_output_port_t, Dynamics>)
                    irt_return_if_bad(dyn.lambda(*this));
        }

        if constexpr (is_detected_v<transition_function_t, Dynamics>)
            irt_return_if_bad(dyn.transition(*this, t, t - mdl.tl, mdl.tn - t));

        if constexpr (is_detected_v<has_input_port_t, Dynamics>) {
            for (auto& elem : dyn.x)
                append_message(*this, elem).clear();
        }

        irt_assert(mdl.tn >= t);

        mdl.tl = t;
        mdl.tn = t + dyn.sigma;
        if (dyn.sigma && mdl.tn == t)
            mdl.tn = std::nextafter(t, t + irt::one);

        sched.reintegrate(mdl, mdl.tn);

        return status::success;
    }

    status make_transition(model& mdl, time t) noexcept
    {
        return dispatch(mdl, [this, &mdl, t]<typename Dynamics>(Dynamics& dyn) {
            return this->make_transition(mdl, dyn, t);
        });
    }

    template<typename Dynamics>
    status make_finalize(model&    mdl,
                         Dynamics& dyn,
                         observer* obs,
                         time      t) noexcept
    {
        if constexpr (is_detected_v<observation_function_t, Dynamics>) {
            if (obs) {
                obs->msg = dyn.observation(t - mdl.tl);
                obs->cb(*obs, mdl.type, mdl.tl, t, observer::status::finalize);
            }
        }

        if constexpr (is_detected_v<finalize_function_t, Dynamics>) {
            irt_return_if_bad(dyn.finalize(*this));
        }

        return status::success;
    }

    /**
     * @brief Finalize and cleanup simulation objects.
     *
     * Clean:
     * - the scheduller nodes
     * - all input/output remaining messages
     * - call the observers' callback to finalize observation
     *
     * This function must be call at the end of the simulation.
     */
    status finalize(time t) noexcept
    {
        model* mdl = nullptr;
        while (models.next(mdl)) {
            observer* obs = nullptr;
            if (is_defined(mdl->obs_id))
                obs = observers.try_to_get(mdl->obs_id);

            auto ret = dispatch(
              *mdl, [this, mdl, obs, t]<typename Dynamics>(Dynamics& dyn) {
                  return this->make_finalize(*mdl, dyn, obs, t);
              });

            irt_return_if_bad(ret);
        }

        return status::success;
    }
};

inline status initialize_source(simulation& sim, source& src) noexcept
{
    return sim.source_dispatch(src, source::operation_type::initialize);
}

inline status update_source(simulation& sim, source& src, double& val) noexcept
{
    if (src.next(val))
        return status::success;

    if (auto ret = sim.source_dispatch(src, source::operation_type::update);
        is_bad(ret))
        return ret;

    return src.next(val) ? status::success : status::source_empty;
}

inline status finalize_source(simulation& sim, source& src) noexcept
{
    return sim.source_dispatch(src, source::operation_type::finalize);
}

inline bool can_alloc_message(const simulation& sim, int alloc_number) noexcept
{
    return sim.message_alloc.can_alloc(alloc_number);
}

inline list_view<message> append_message(simulation& sim,
                                         input_port& port) noexcept
{
    return list_view<message>(sim.message_alloc, port);
}

inline list_view_const<message> get_message(const simulation& sim,
                                            const input_port  port) noexcept
{
    return list_view_const<message>(sim.message_alloc, port);
}

inline list_view<node> append_node(simulation& sim, output_port& port) noexcept
{
    return list_view<node>(sim.node_alloc, port);
}

inline list_view_const<node> get_node(const simulation& sim,
                                      const output_port port) noexcept
{
    return list_view_const<node>(sim.node_alloc, port);
}

inline list_view_const<record> get_archive(const simulation& sim,
                                           const u64         id) noexcept
{
    return list_view_const<record>(sim.record_alloc, id);
}

inline list_view<record> append_archive(simulation& sim, u64& id) noexcept
{
    return list_view<record>(sim.record_alloc, id);
}

inline bool can_alloc_node(const simulation& sim, int alloc_number) noexcept
{
    return sim.node_alloc.can_alloc(alloc_number);
}

inline bool can_alloc_dated_message(const simulation& sim,
                                    int               alloc_number) noexcept
{
    return sim.dated_message_alloc.can_alloc(alloc_number);
}

inline list_view<dated_message> append_dated_message(simulation& sim,
                                                     u64&        id) noexcept
{
    return list_view<dated_message>(sim.dated_message_alloc, id);
}

inline list_view_const<dated_message> get_dated_message(const simulation& sim,
                                                        const u64 id) noexcept
{
    return list_view_const<dated_message>(sim.dated_message_alloc, id);
}

inline status send_message(simulation&  sim,
                           output_port& p,
                           real         r1,
                           real         r2,
                           real         r3) noexcept
{
    auto list = append_node(sim, p);
    auto it   = list.begin();
    auto end  = list.end();

    while (it != end) {
        auto* mdl = sim.models.try_to_get(it->model);
        if (!mdl) {
            it = list.erase(it);
        } else {
            irt_return_if_fail(sim.emitting_output_ports.can_alloc(1),
                               status::simulation_not_enough_message);

            auto& output_message  = sim.emitting_output_ports.emplace_back();
            output_message.msg[0] = r1;
            output_message.msg[1] = r2;
            output_message.msg[2] = r3;
            output_message.model  = it->model;
            output_message.port   = it->port_index;

            ++it;
        }
    }

    return status::success;
}

/*****************************************************************************
 *
 * Containers implementation
 *
 ****************************************************************************/

// template<typename T>
// class vector;

template<typename T>
inline vector<T>::vector(i32 capacity) noexcept
{
    if (capacity > 0) {
        m_data     = reinterpret_cast<T*>(g_alloc_fn(capacity * sizeof(T)));
        m_capacity = capacity;
        m_size     = 0;
    }
}

template<typename T>
inline vector<T>::vector(i32 capacity, i32 size) noexcept
{
    static_assert(std::is_default_constructible_v<T>,
                  "init with a default size need a default constructor");

    capacity = std::max(capacity, size);

    if (capacity > 0) {
        m_data     = reinterpret_cast<T*>(g_alloc_fn(capacity * sizeof(T)));
        m_capacity = capacity;
        m_size     = size;
    }
}

template<typename T>
inline vector<T>::~vector() noexcept
{
    destroy();
}

template<typename T>
constexpr vector<T>::vector(const vector& other) noexcept
  : m_data(nullptr)
  , m_size(0)
  , m_capacity(0)
{
    operator=(other);
}

template<typename T>
constexpr vector<T>& vector<T>::operator=(const vector& other) noexcept
{
    clear();

    resize(other.m_size);
    std::copy_n(other.m_data, other.m_size, m_data);
    return *this;
}

template<typename T>
constexpr vector<T>::vector(vector&& other) noexcept
  : m_data(other.m_data)
  , m_size(other.m_size)
  , m_capacity(other.m_capacity)
{
    other.m_data     = nullptr;
    other.m_size     = 0;
    other.m_capacity = 0;
}

template<typename T>
constexpr vector<T>& vector<T>::operator=(vector&& other) noexcept
{
    destroy();

    m_data           = other.m_data;
    m_size           = other.m_size;
    m_capacity       = other.m_capacity;
    other.m_data     = nullptr;
    other.m_size     = 0;
    other.m_capacity = 0;

    return *this;
}

template<typename T>
inline void vector<T>::destroy() noexcept
{
    clear();

    if (m_data)
        g_free_fn(m_data);

    m_data     = nullptr;
    m_size     = 0;
    m_capacity = 0;
}

template<typename T>
constexpr void vector<T>::clear() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        for (i32 i = 0; i != m_size; ++i)
            data()[i].~T();
    }

    m_size = 0;
}

template<typename T>
void vector<T>::resize(i32 size) noexcept
{
    static_assert(std::is_default_constructible_v<T>,
                  "init with a default size need a default constructor");

    if (size > m_capacity)
        reserve(compute_new_capacity(size));

    for (i32 i = 0; i != size; ++i)
        new (&(m_data[i])) T{};

    m_size = size;
}

template<typename T>
void vector<T>::reserve(i32 new_capacity) noexcept
{
    irt_assert(new_capacity > 0);

    if (new_capacity > m_capacity) {
        T* new_data =
          reinterpret_cast<T*>(g_alloc_fn(new_capacity * sizeof(T)));

        if constexpr (std::is_copy_assignable_v<T> ||
                      std::is_copy_constructible_v<T> ||
                      std::is_nothrow_copy_constructible_v<T>)
            std::copy_n(m_data, m_size, new_data);
        else if constexpr (std::is_move_assignable_v<T> ||
                           std::is_move_constructible_v<T> ||
                           std::is_nothrow_move_assignable_v<T>)
            for (i32 i = 0; i != size; ++i)
                new_data[i] = std::move(m_data[i]);

        g_free_fn(m_data);

        m_data     = new_data;
        m_capacity = new_capacity;
    }
}

template<typename T>
constexpr T* vector<T>::data() noexcept
{
    return m_data;
}

template<typename T>
constexpr const T* vector<T>::data() const noexcept
{
    return m_data;
}

template<typename T>
constexpr typename vector<T>::reference vector<T>::front() noexcept
{
    irt_assert(m_size > 0);
    return m_data[0];
}

template<typename T>
constexpr typename vector<T>::const_reference vector<T>::front() const noexcept
{
    irt_assert(m_size > 0);
    return m_data[0];
}

template<typename T>
constexpr typename vector<T>::reference vector<T>::back() noexcept
{
    irt_assert(m_size > 0);
    return m_data[m_size - 1];
}

template<typename T>
constexpr typename vector<T>::const_reference vector<T>::back() const noexcept
{
    irt_assert(m_size > 0);
    return m_data[m_size - 1];
}

template<typename T>
constexpr typename vector<T>::reference vector<T>::operator[](
  const index_type index) noexcept
{
    irt_assert(index >= 0 && index < m_size);

    return data()[index];
}

template<typename T>
constexpr typename vector<T>::const_reference vector<T>::operator[](
  const index_type index) const noexcept
{
    irt_assert(index >= 0 && index < m_size);

    return data()[index];
}

template<typename T>
constexpr typename vector<T>::iterator vector<T>::begin() noexcept
{
    return data();
}

template<typename T>
constexpr typename vector<T>::const_iterator vector<T>::begin() const noexcept
{
    return data();
}

template<typename T>
constexpr typename vector<T>::iterator vector<T>::end() noexcept
{
    return data() + m_size;
}

template<typename T>
constexpr typename vector<T>::const_iterator vector<T>::end() const noexcept
{
    return data() + m_size;
}

template<typename T>
constexpr sz vector<T>::size() const noexcept
{
    return static_cast<sz>(m_size);
}

template<typename T>
constexpr i32 vector<T>::ssize() const noexcept
{
    return m_size;
}

template<typename T>
constexpr sz vector<T>::capacity() const noexcept
{
    return static_cast<sz>(m_capacity);
}

template<typename T>
constexpr bool vector<T>::empty() const noexcept
{
    return m_size == 0;
}

template<typename T>
constexpr bool vector<T>::full() const noexcept
{
    return m_size >= m_capacity;
}

template<typename T>
constexpr bool vector<T>::can_alloc(int number) const noexcept
{
    return m_capacity - m_size >= number;
}

template<typename T>
constexpr i32 vector<T>::find(const T& t) const noexcept
{
    for (i32 i = 0, e = ssize(); i != e; ++i)
        if (m_data[i] == t)
            return i;

    return m_size;
}

template<typename T>
template<typename... Args>
constexpr typename vector<T>::reference vector<T>::emplace_back(
  Args&&... args) noexcept
{
    if (m_size >= m_capacity)
        reserve(compute_new_capacity(m_size + 1));

    new (&(data()[m_size])) T(std::forward<Args>(args)...);

    ++m_size;

    return data()[m_size - 1];
}

template<typename T>
constexpr void vector<T>::pop_back() noexcept
{
    if (m_size) {
        if constexpr (std::is_trivially_destructible_v<T>)
            data()[m_size - 1].~T();

        --m_size;
    }
}

template<typename T>
constexpr void vector<T>::swap_pop_back(index_type index) noexcept
{
    irt_assert(index < m_size);

    if (index == m_size - 1) {
        pop_back();
    } else {
        if constexpr (std::is_trivially_destructible_v<T>) {
            data()[index] = data()[m_size - 1];
            pop_back();
        } else {
            using std::swap;

            swap(data()[index], data()[m_size - 1]);
            pop_back();
        }
    }
}

template<typename T>
constexpr void vector<T>::erase(iterator it) noexcept
{
    irt_assert(it >= data() && it < data() + m_size);

    if constexpr (std::is_trivially_copyable_v<T>) {
        const ptrdiff_t off = it - data();
        std::memmove(data() + off,
                     data() + off + 1,
                     (static_cast<sz>(m_size) - static_cast<sz>(off) - 1) *
                       sizeof(T));
        --m_size;
    } else if (std::is_nothrow_move_constructible_v<T>) {
        (*it).~T();

        auto prev = it++;
        for (; it != end(); ++it, ++prev)
            (*prev) = std::move(*it);

        pop_back();
    } else if (std::is_nothrow_constructible_v<T>) {
        (*it).~T();

        auto prev = it++;
        for (; it != end(); ++it, ++prev)
            (*prev) = (*it);

        pop_back();
    }
}

template<typename T>
constexpr void vector<T>::erase(iterator first, iterator last) noexcept
{
    irt_assert(first >= data() && first < data() + m_size && last > first &&
               last <= data() + m_size);

    const ptrdiff_t count = last - first;

    if constexpr (std::is_trivially_copyable_v<T>) {
        const ptrdiff_t off = first - data();
        std::memmove(data() + off,
                     data() + off + count,
                     (static_cast<sz>(m_size) - static_cast<sz>(off) -
                      static_cast<sz>(count)) *
                       sizeof(T));
        m_size -= static_cast<i32>(count);
    } else if (std::is_nothrow_move_constructible_v<T>) {
        for (auto jt = first; jt < last; ++jt)
            (*jt).~T();

        auto prev = first;
        first     = last;
        for (; last != end(); ++first, ++prev)
            (*prev) = std::move(*first);

        m_size -= static_cast<i32>(count);
    } else if (std::is_nothrow_constructible_v<T>) {
        (*first).~T();

        auto prev = first;
        first     = last;
        for (; last != end(); ++first, ++prev)
            (*prev) = *first;

        m_size -= static_cast<i32>(count);

        for (ptrdiff_t i = 0; i < count; ++i)
            pop_back();
    }
}

template<typename T>
i32 vector<T>::compute_new_capacity(i32 size) const
{
    i32 new_capacity = m_capacity ? (m_capacity + m_capacity / 2) : 8;
    return new_capacity > size ? new_capacity : size;
}

// template<size_t length = 8>
// class small_string

template<sz length>
constexpr small_string<length>::small_string() noexcept
{
    clear();
}

template<sz length>
constexpr small_string<length>::small_string(
  const small_string<length>& str) noexcept
{
    std::copy_n(str.m_buffer, str.m_size, m_buffer);
    m_buffer[str.m_size] = '\0';
    m_size               = str.m_size;
}

template<sz length>
constexpr small_string<length>::small_string(
  small_string<length>&& str) noexcept
{
    std::copy_n(str.m_buffer, str.m_size, m_buffer);
    m_buffer[str.m_size] = '\0';
    m_size               = str.m_size;
    str.clear();
}

template<sz length>
constexpr small_string<length>& small_string<length>::operator=(
  const small_string<length>& str) noexcept
{
    if (&str != this) {
        std::copy_n(str.m_buffer, str.m_size, m_buffer);
        m_buffer[str.m_size] = '\0';
        m_size               = str.m_size;
    }

    return *this;
}

template<sz length>
constexpr small_string<length>& small_string<length>::operator=(
  small_string<length>&& str) noexcept
{
    if (&str != this) {
        std::copy_n(str.m_buffer, str.m_size, m_buffer);
        m_buffer[str.m_size] = '\0';
        m_size               = str.m_size;
    }

    return *this;
}

template<sz length>
constexpr small_string<length>& small_string<length>::operator=(
  const char* str) noexcept
{
    if (m_buffer != str) {
        std::strncpy(m_buffer, str, length - 1);
        m_buffer[length - 1] = '\0';
        m_size               = static_cast<u8>(std::strlen(m_buffer));
    }
    return *this;
}

template<sz length>
constexpr small_string<length>& small_string<length>::operator=(
  const std::string_view str) noexcept
{
    assign(str);

    return *this;
}

template<sz length>
constexpr small_string<length>::small_string(const char* str) noexcept
{
    std::strncpy(m_buffer, str, length - 1);
    m_buffer[length - 1] = '\0';
    m_size               = static_cast<u8>(std::strlen(m_buffer));
}

template<sz length>
constexpr small_string<length>::small_string(
  const std::string_view str) noexcept
{
    assign(str);
}

template<sz length>
void small_string<length>::resize(size_type size) noexcept
{
    m_size               = size > length ? length : size;
    m_buffer[m_size - 1] = '\0';
}

template<sz length>
constexpr bool small_string<length>::empty() const noexcept
{
    constexpr unsigned char zero{ 0 };

    return zero == m_size;
}

template<sz length>
constexpr sz small_string<length>::size() const noexcept
{
    return m_size;
}
template<sz length>
constexpr sz small_string<length>::capacity() const noexcept
{
    return length;
}

template<sz length>
constexpr void small_string<length>::assign(const std::string_view str) noexcept
{
    const auto copy_length = std::min(str.size(), length - 1);

    std::memcpy(m_buffer, str.data(), copy_length);
    m_buffer[copy_length] = '\0';

    m_size = static_cast<u8>(copy_length);
}

template<sz length>
constexpr std::string_view small_string<length>::sv() const noexcept
{
    return { m_buffer, m_size };
}

template<sz length>
constexpr void small_string<length>::clear() noexcept
{
    std::fill_n(m_buffer, length, '\0');
    m_size = 0;
}

template<sz length>
constexpr typename small_string<length>::reference
small_string<length>::operator[](const size_type index) noexcept
{
    irt_assert(index < m_size);

    return m_buffer[index];
}

template<sz length>
constexpr typename small_string<length>::const_reference
small_string<length>::operator[](const size_type index) const noexcept
{
    irt_assert(index < m_size);

    return m_buffer[index];
}

template<sz length>
constexpr const char* small_string<length>::c_str() const noexcept
{
    return m_buffer;
}

template<sz length>
constexpr typename small_string<length>::iterator
small_string<length>::begin() noexcept
{
    return m_buffer;
}

template<sz length>
constexpr typename small_string<length>::iterator
small_string<length>::end() noexcept
{
    return m_buffer + m_size;
}

template<sz length>
constexpr typename small_string<length>::const_iterator
small_string<length>::begin() const noexcept
{
    return m_buffer;
}

template<sz length>
constexpr typename small_string<length>::const_iterator
small_string<length>::end() const noexcept
{
    return m_buffer + m_size;
}

template<sz length>
constexpr bool small_string<length>::operator==(
  const small_string<length>& rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs.m_buffer, length) == 0;
}

template<sz length>
constexpr bool small_string<length>::operator!=(
  const small_string<length>& rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs.m_buffer, length) != 0;
}

template<sz length>
constexpr bool small_string<length>::operator>(
  const small_string<length>& rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs.m_buffer, length) > 0;
}

template<sz length>
constexpr bool small_string<length>::operator<(
  const small_string<length>& rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs.m_buffer, length) < 0;
}

template<sz length>
constexpr bool small_string<length>::operator==(const char* rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs, length) == 0;
}

template<sz length>
constexpr bool small_string<length>::operator!=(const char* rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs, length) != 0;
}

template<sz length>
constexpr bool small_string<length>::operator>(const char* rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs, length) > 0;
}

template<sz length>
constexpr bool small_string<length>::operator<(const char* rhs) const noexcept
{
    return std::strncmp(m_buffer, rhs, length) < 0;
}

} // namespace irt

#endif
