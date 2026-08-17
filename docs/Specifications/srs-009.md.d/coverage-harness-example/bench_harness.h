#pragma once
#include <stdio.h>
#include <time.h>

static inline double bench_now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static inline void bench_report(const char *name, long iterations, double total_ns) {
  double ns_per_op = total_ns / (double)iterations;
  double ops_per_sec = 1e9 / ns_per_op;
  printf("%-40s %10ld iters  %12.1f ns/op  %14.0f ops/sec\n", name, iterations, ns_per_op, ops_per_sec);
}

/* `warmup` iterations run untimed first, then `iterations` are timed.
 * Insert-shaped benchmarks pass warmup=0 (see srs-009.md benchmark-strategy
 * rule 3) since warming up would already do the timed region's work. */
#define BENCH(name, iterations, warmup, body)               \
  do {                                                      \
    for (long _bi = 0; _bi < (warmup); _bi++) { body; }      \
    double _start = bench_now_ns();                          \
    for (long _bi = 0; _bi < (iterations); _bi++) { body; }   \
    double _end = bench_now_ns();                             \
    bench_report((name), (iterations), _end - _start);         \
  } while (0)
