#ifndef PRIMECOUNT_H
#define PRIMECOUNT_H

#include <stdint.h>

#define PRIMECOUNT_VERSION "0.1"
#define PRIMECOUNT_MAJOR_VERSION 0
#define PRIMECOUNT_MINOR_VERSION 1
#define PRIMECOUNT_YEAR "2013"

namespace primecount {

enum {
  /// Uses all CPU cores, do not modify.
  MAX_THREADS = -1
};

int64_t pi(int64_t x, int threads = MAX_THREADS);

int64_t pi_primesieve(int64_t x, int threads = MAX_THREADS);
int64_t pi_legendre(int64_t x, int threads = MAX_THREADS);
int64_t pi_meissel(int64_t x, int threads = MAX_THREADS);
int64_t pi_lehmer(int64_t x, int threads = MAX_THREADS);

int64_t phi(int64_t x, int64_t a, int threads = MAX_THREADS);

int64_t nth_prime(int64_t n, int threads = MAX_THREADS);

int64_t Li(int64_t);
int64_t Li_inverse(int64_t);

} // namespace primecount

#endif
