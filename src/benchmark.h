#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h>
#include "matrix.h"

// Function prototype for benchmarking matrix multiplication
void benchmark(void (*multiply)(benchmark_args_t* args), benchmark_args_t* args);

// Function prototype for writing benchmark results to a file
void writeToFile(benchmark_args_t* args);

#endif /* BENCHMARK_H */
