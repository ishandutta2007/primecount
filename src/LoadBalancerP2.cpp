///
/// @file  LoadBalancerP2.cpp
/// @brief This load balancer assigns work to the threads in the
///        computation of the 2nd partial sieve function.
///        It is used by the P2(x, a) and B(x, y) functions.
///
/// Copyright (C) 2022 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include "LoadBalancerP2.hpp"
#include "primecount-internal.hpp"
#include "imath.hpp"
#include "min.hpp"

#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace primecount {

/// We need to sieve [sqrt(x), sieve_limit[
LoadBalancerP2::LoadBalancerP2(maxint_t x,
                               int64_t sieve_limit,
                               int threads,
                               bool is_print) :
  low_(isqrt(x)),
  sieve_limit_(sieve_limit),
  precision_(get_status_precision(x)),
  is_print_(is_print)
{
  low_ = min(low_, sieve_limit_);
  int64_t dist = sieve_limit_ - low_;

  // These load balancing settings work well on my
  // dual-socket AMD EPYC 7642 server with 192 CPU cores.
  min_thread_dist_ = 1 << 23;
  int max_threads = (int) std::pow(sieve_limit_, 1 / 3.7);
  threads = std::min(threads, max_threads);
  threads_ = ideal_num_threads(dist, threads, min_thread_dist_);
  lock_.init(threads_);

  // Using more chunks per thread improves load
  // balancing but also adds some overhead.
  int64_t chunks_per_thread = 8;
  thread_dist_ = dist / (threads_ * chunks_per_thread);
  thread_dist_ = max(min_thread_dist_, thread_dist_);
}

int LoadBalancerP2::get_threads() const
{
  return threads_;
}

/// The thread needs to sieve [low, high[
bool LoadBalancerP2::get_work(int64_t& low, int64_t& high)
{
  LockGuard lockGuard(lock_);
  print_status();

  // Calculate the remaining sieving distance
  low_ = min(low_, sieve_limit_);
  int64_t dist = sieve_limit_ - low_;

  // When a single thread is used (and printing is
  // disabled) we can set thread_dist to the entire
  // sieving distance as load balancing is only
  // useful for multi-threading.
  if (threads_ == 1)
  {
    if (!is_print_)
      thread_dist_ = dist;
  }
  else
  {
    // Ensure that the thread initialization, i.e. the
    // computation of PrimePi(low), uses at most 10%
    // of the entire thread computation.
    // Since PrimePi(low) uses O(low^(2/3)/log(low)^2) time,
    // sieving a distance of n = low^(2/3) * 5 uses
    // O(n log log n) time, which is more than 10x more.
    double low13 = std::cbrt(low_);
    int64_t n = (int64_t) (low13 * low13 * 5);
    min_thread_dist_ = std::max(min_thread_dist_, n);
    thread_dist_ = max(min_thread_dist_, thread_dist_);

    // Reduce the thread distance near to end to keep all
    // threads busy until the computation finishes.
    int64_t max_thread_dist = dist / threads_;
    if (thread_dist_ > max_thread_dist)
      thread_dist_ = max(min_thread_dist_, max_thread_dist);
  }

  low = low_;
  low_ += thread_dist_;
  low_ = min(low_, sieve_limit_);
  high = low_;

  return low < sieve_limit_;
}

void LoadBalancerP2::print_status()
{
  if (is_print_)
  {
    double time = get_time();
    double old = time_;
    double threshold = 0.1;

    if ((time - old) >= threshold)
    {
      time_ = time;
      std::cout << "\rStatus: " << std::fixed << std::setprecision(precision_)
                << get_percent(low_, sieve_limit_) << '%' << std::flush;
    }
  }
}

} // namespace
