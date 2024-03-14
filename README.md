# Matrix Multiplication Benchmark

This lab aims to benchmark different methods of matrix multiplication in a multi-threaded environment. Three methods are implemented and compared:

1. Matrix multiplication using a thread per matrix.
2. Matrix multiplication using a thread per row.
3. Matrix multiplication using a thread per element.

## Getting Started

### Prerequisites

- C compiler (e.g., GCC)
- pthread library

### Installation

1. Clone the repository:
```
git clone <repository_url>
```

2. Compile the project:
```
make all
```

### Usage

To run the benchmark, execute the compiled binary with the following command:
```
./matMultp <input_file_A> <input_file_B> <output_file_prefix>
```

Replace `<input_file_A>` and `<input_file_B>` with the paths to the input matrix files (without extension). `<output_file_prefix>` is the prefix for the output files containing benchmarking results.

For example:
```
./matMultp matrix_A matrix_B result
```

This command will read matrices from `matrix_A.txt` and `matrix_B.txt`, perform matrix multiplication using different methods, and write the benchmarking results to files named `result_per_matrix.txt`, `result_per_row.txt`, and `result_per_element.txt`.

### Testing
To run the tests, execute the `test_cases.sh` script located in the project root directory. This script automatically compiles the program, runs tests from the `test` directory, and generates benchmarking results.
```
./test_cases.sh
```

## File Structure

- `src/benchmark.h`: Header file containing function declarations for benchmarking.
- `src/benchmark.c`: Source file containing function definitions for benchmarking.
- `src/matrix.h`: Header file containing function declarations for matrix multiplication operations.
- `src/matrix.c`: Source file containing function definitions for matrix multiplication operations.
- `src/main.c`: Source file containing the main function to run the program.
- `test/` : The directory containing the test cases.
- `Makefile`: Makefile for compiling the shell program.
- `README.md`: This file providing information about the program.

