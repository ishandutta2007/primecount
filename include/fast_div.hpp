///
/// @file  fast_div.hpp
/// @brief Integer division of small types is much faster than integer
///        division of large types on most CPUs. The fast_div(x, y)
///        function tries to take advantage of this by casting x and y
///        to smaller types (if possible) before doing the division.
///
///        If FAST_DIV32 is defined we try to use 32-bit division
///        instead of 64-bit division whenever possible. This performs
///        best on CPUs where 64-bit division is much slower than
///        32-bit division (most CPUs before 2020).
///
///        On some new CPUs (such as Intel Cannonlake & IBM POWER 9)
///        64-bit integer division has been improved significantly and
///        runs as fast as 32-bit integer division. For such CPUs it
///        is best to disable FAST_DIV32.
///
/// Copyright (C) 2020 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef FAST_DIV_HPP
#define FAST_DIV_HPP

#include <int128_t.hpp>

#include <cassert>
#include <limits>
#include <type_traits>

namespace primecount {

/// Returns the next smaller integer type.
/// fastdiv<uint64_t>::type -> uint32_t.
/// fastdiv<uint128_t>::type -> uint64_t.
///
template <typename T>
struct fastdiv
{
  typedef typename std::conditional<sizeof(T) / 2 <= sizeof(uint32_t), uint32_t,
          typename std::conditional<sizeof(T) / 2 <= sizeof(uint64_t), uint64_t,
          T>::type>::type type;
};

/// If FAST_DIV32 is defined we:
/// 1) Optimize  64-bit integer division.
/// 2) Optimize 128-bit integer division.
///
#if defined(FAST_DIV32)

/// Optimize  (64-bit / 64-bit) =  64-bit.
/// Optimize (128-bit / 64-bit) = 128-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) == sizeof(Y)), X>::type
fast_div(X x, Y y)
{
  static_assert(prt::is_integral<X>::value &&
                prt::is_integral<Y>::value,
                "fast_div(x, y): types must be integral");

  using fastdiv_t = typename fastdiv<X>::type;

  if (x <= std::numeric_limits<fastdiv_t>::max() &&
      y <= std::numeric_limits<fastdiv_t>::max())
  {
    return (fastdiv_t) x / (fastdiv_t) y;
  }

  return x / y;
}

/// Optimize  (64-bit / 32-bit) =  64-bit.
/// Optimize (128-bit / 64-bit) = 128-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) > sizeof(Y)), X>::type
fast_div(X x, Y y)
{
  static_assert(prt::is_integral<X>::value &&
                prt::is_integral<Y>::value,
                "fast_div(x, y): types must be integral");

  using fastdiv_t = typename fastdiv<X>::type;

  if (x <= std::numeric_limits<fastdiv_t>::max())
    return (fastdiv_t) x / (fastdiv_t) y;

  return x / y;
}

#else
/// If FAST_DIV32 is not defined we:
/// 1) Use the regular 64-bit integer division.
/// 2) Optimize 128-bit integer division.

/// Regular (32-bit / 32-bit) = 32-bit.
/// Regular (64-bit / 64-bit) = 64-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) == sizeof(Y) &&
                         sizeof(X) <= sizeof(uint64_t)), X>::type
fast_div(X x, Y y)
{
  // Unsigned integer division is usually
  // faster than signed integer division.
  using fastdiv_t = typename std::make_unsigned<X>::type;
  return (fastdiv_t) x / (fastdiv_t) y;
}

/// Regular (64-bit / 32-bit) = 64-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) > sizeof(Y) &&
                         sizeof(X) <= sizeof(uint64_t)), X>::type
fast_div(X x, Y y)
{
  using fastdiv_t = typename std::make_unsigned<X>::type;
  return (fastdiv_t) x / (fastdiv_t) y;
}

/// Optimize (128-bit / 64-bit) = 128-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) > sizeof(Y) &&
                         sizeof(X) > sizeof(uint64_t)), X>::type
fast_div(X x, Y y)
{
  static_assert(prt::is_integral<X>::value &&
                prt::is_integral<Y>::value,
                "fast_div(x, y): types must be integral");

  using fastdiv_t = typename fastdiv<X>::type;

  if (x <= std::numeric_limits<fastdiv_t>::max())
    return (fastdiv_t) x / (fastdiv_t) y;

  return x / y;
}

/// Optimize (128-bit / 128-bit) = 128-bit.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) == sizeof(Y) &&
                         sizeof(X) > sizeof(uint64_t)), X>::type
fast_div(X x, Y y)
{
  static_assert(prt::is_integral<X>::value &&
                prt::is_integral<Y>::value,
                "fast_div(x, y): types must be integral");

  using fastdiv_t = typename fastdiv<X>::type;

  if (x <= std::numeric_limits<fastdiv_t>::max() &&
      y <= std::numeric_limits<fastdiv_t>::max())
  {
    return (fastdiv_t) x / (fastdiv_t) y;
  }

  return x / y;
}

#endif

/// Optimize (128-bit / 64-bit) = 64-bit, for x64.
template <typename X, typename Y>
typename std::enable_if<(sizeof(X) == sizeof(uint64_t) * 2 &&
                         sizeof(Y) <= sizeof(uint64_t)), uint64_t>::type
fast_div64(X x, Y y)
{
#if defined(__x86_64__) && \
   (defined(__GNUC__) || defined(__clang__))

  // primecount does not need signed division so 
  // we use the unsigned division instruction further
  // down as DIV is usually faster than IDIV.
  assert(x >= 0 && y > 0);

  uint64_t x0 = (uint64_t) x;
  uint64_t x1 = ((uint64_t*) &x)[1];
  uint64_t d = y;

  // (128-bit / 64-bit) = 64-bit.
  // When we know the result fits into 64-bit (even
  // though the numerator is 128-bit) we can use the divq
  // instruction instead of doing a full 128-bit division.
  __asm__("divq %[divider]"
          : "+a"(x0), "+d"(x1) : [divider] "r"(d));

  return x0;
#else
  return (uint64_t) fast_div(x, y);
#endif
}

/// (?-bit / 64-bit) = 64-bit
template <typename X, typename Y>
typename std::enable_if<!(sizeof(X) == sizeof(uint64_t) * 2 &&
                          sizeof(Y) <= sizeof(uint64_t)), uint64_t>::type
fast_div64(X x, Y y)
{
  return (uint64_t) fast_div(x, y);
}

} // namespace

#endif
