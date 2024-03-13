#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Struct definition for a matrix
typedef struct 
{
    int rows;       // Number of rows in the matrix
    int cols;       // Number of columns in the matrix
    int** mat;      // Pointer to the matrix data
} matrix_t;

// Struct definition for thread arguments used in matrix multiplication
typedef struct 
{
    int currentRow;     // Current row index for computation
    int currentCol;     // Current column index for computation
    matrix_t* A;        // Pointer to the first input matrix
    matrix_t* B;        // Pointer to the second input matrix
    matrix_t* C;        // Pointer to the output matrix
} thread_args_t;

// Struct definition for benchmarking arguments
typedef struct
{
    char* outputFile;   // Output file path
    int threadCount;    // Number of threads for computation
    double time;        // Time taken for computation
    matrix_t* A;        // Pointer to the first input matrix
    matrix_t* B;        // Pointer to the second input matrix
    matrix_t* C;        // Pointer to the output matrix
} benchmark_args_t;

// Function prototypes
void readMatrix(FILE *file, matrix_t* mat);                // Function to read a matrix from a file
void *multiplyMatrixThread(void *args);                    // Function for matrix multiplication using threads
void *multiplyRowThread(void *args);                       // Function for matrix multiplication by rows using threads
void *multiplyElementThread(void *args);                   // Function for matrix multiplication by elements using threads
void multiplyMatrix(benchmark_args_t* bench_args);         // Function to perform matrix multiplication
void multiplyRows(benchmark_args_t* bench_args);           // Function to perform matrix multiplication by rows
void multiplyElements(benchmark_args_t* bench_args);       // Function to perform matrix multiplication by elements

#endif /* MATRIX_H */
