///
/// @file   api.cpp
/// @brief  primesieve C++ API.
///         Contains the implementations of the functions declared
///         in the primesieve.hpp header file.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "primesieve.hpp"
#include "primesieve/config.hpp"
#include "primesieve/CpuInfo.hpp"
#include "primesieve/pmath.hpp"
#include "primesieve/PrimeSieve.hpp"
#include "primesieve/ParallelSieve.hpp"

#include <stdint.h>
#include <cstddef>
#include <limits>
#include <string>

using std::size_t;

namespace {

int sieve_size = 0;

int num_threads = 0;

}

namespace primesieve {

uint64_t nth_prime(int64_t n, uint64_t start)
{
  ParallelSieve ps;
  return ps.nthPrime(n, start);
}

uint64_t count_primes(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_PRIMES);
  return ps.getCount(0);
}

uint64_t count_twins(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_TWINS);
  return ps.getCount(1);
}

uint64_t count_triplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_TRIPLETS);
  return ps.getCount(2);
}

uint64_t count_quadruplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_QUADRUPLETS);
  return ps.getCount(3);
}

uint64_t count_quintuplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_QUINTUPLETS);
  return ps.getCount(4);
}

uint64_t count_sextuplets(uint64_t start, uint64_t stop)
{
  ParallelSieve ps;
  ps.sieve(start, stop, COUNT_SEXTUPLETS);
  return ps.getCount(5);
}

void print_primes(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_PRIMES);
}

void print_twins(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_TWINS);
}

void print_triplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_TRIPLETS);
}

void print_quadruplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_QUADRUPLETS);
}

void print_quintuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_QUINTUPLETS);
}

void print_sextuplets(uint64_t start, uint64_t stop)
{
  PrimeSieve ps;
  ps.sieve(start, stop, PRINT_SEXTUPLETS);
}

int get_num_threads()
{
  if (num_threads)
    return num_threads;
  else
    return ParallelSieve::getMaxThreads();
}

void set_num_threads(int threads)
{
  num_threads = inBetween(1, threads, ParallelSieve::getMaxThreads());
}

uint64_t get_max_stop()
{
  return std::numeric_limits<uint64_t>::max();
}

std::string primesieve_version()
{
  return PRIMESIEVE_VERSION;
}

void set_sieve_size(int size)
{
  sieve_size = inBetween(16, size, 8192);
}

int get_sieve_size()
{
  // User specified sieve size
  if (sieve_size)
    return sieve_size;

  // The CPU cache hierarchy has become very complex and hence accurately
  // detecting the private L2 cache size per core has become very
  // difficult. The CPU information returned by the operating system is
  // also unreliable, for new CPUs or niche CPUs the CPU info is
  // frequently incorrect. Therefore, we don't want to use a sieve size
  // that matches the CPU's L2 cache size because if we guessed wrong
  // and our sieve array is too large the performance will deteriorate
  // significantly due to excessive cache misses. In order to prevent
  // this issue, we use a smaller sieve size that is 8x larger than the
  // L1 cache size. This sieve size provides great performance on every
  // CPU (architecture) that I have tested.
  // https://github.com/kimwalisch/primesieve/issues/103
  // https://github.com/kimwalisch/primesieve/issues/96
  if (cpuInfo.hasL1Cache() &&
      cpuInfo.hasL2Cache())
  {
    // Convert bytes to KiB
    size_t l1Size = cpuInfo.l1CacheBytes() >> 10;
    size_t l2Size = cpuInfo.l2CacheBytes() >> 10;
    size_t maxSize = l2Size / 2;

    if (cpuInfo.hasL2Sharing() &&
        cpuInfo.l2Sharing() > 2)
      maxSize = l2Size / cpuInfo.l2Sharing();

    maxSize = std::max(l1Size, maxSize);
    size_t size = std::min(l1Size * 8, maxSize);
    size = inBetween(16, size, 8192);
    return (int) size;
  }

  if (cpuInfo.hasL1Cache())
  {
    // Convert bytes to KiB
    size_t l1Size = cpuInfo.l1CacheBytes() >> 10;
    l1Size = inBetween(16, l1Size, 8192);
    return (int) l1Size;
  }
  else
  {
    // Default sieve size in KiB
    size_t l1Size = config::L1D_CACHE_BYTES >> 10;
    size_t size = l1Size * 8;
    size = inBetween(16, size, 8192);
    return (int) size;
  }
}

} // namespace
