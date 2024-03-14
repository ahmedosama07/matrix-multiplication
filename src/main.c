#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "matrix.h"
#include "benchmark.h"

#define MAX_SIZE 50

int main(int argc, char *argv[]) {
    // File names
    char *inputFileA = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *inputFileB = (char*)malloc(sizeof(char) * MAX_SIZE);
    
    // Matrices
    matrix_t A;
    matrix_t B;

    // Benchmarking arguments for matrix multiplication per matrix
    matrix_t C_mat;
    benchmark_args_t matArgs;
    matArgs.A = &A;
    matArgs.B = &B;
    matArgs.C = &C_mat;
    matArgs.outputFile = (char*)malloc(sizeof(char) * MAX_SIZE);

    // Benchmarking arguments for matrix multiplication per row
    matrix_t C_row;
    benchmark_args_t rowArgs;
    rowArgs.A = &A;
    rowArgs.B = &B;
    rowArgs.C = &C_row;
    rowArgs.outputFile = (char*)malloc(sizeof(char) * MAX_SIZE);

    // Benchmarking arguments for matrix multiplication per element
    matrix_t C_elem;
    benchmark_args_t elemArgs;
    elemArgs.A = &A;
    elemArgs.B = &B;
    elemArgs.C = &C_elem;
    elemArgs.outputFile = (char*)malloc(sizeof(char) * MAX_SIZE);
    
    // Check for custom arguments
    if (argc == 4) {
        strcpy(inputFileA, argv[1]);
        strcat(inputFileA, ".txt");
        strcpy(inputFileB, argv[2]);
        strcat(inputFileB, ".txt");
        strcpy(matArgs.outputFile, argv[3]);
        strcat(matArgs.outputFile, "_per_matrix.txt");
        strcpy(rowArgs.outputFile, argv[3]);
        strcat(rowArgs.outputFile, "_per_row.txt");
        strcpy(elemArgs.outputFile, argv[3]);
        strcat(elemArgs.outputFile, "_per_element.txt");
    }
    else
    {
        strcpy(inputFileA, "a.txt");
        strcpy(inputFileB, "b.txt");
        strcpy(matArgs.outputFile, "c_per_matrix.txt");
        strcpy(rowArgs.outputFile, "c_per_row.txt");
        strcpy(elemArgs.outputFile, "c_per_element.txt");
    }
    
    // Open input files
    FILE *fileA = fopen(inputFileA, "r");
    FILE *fileB = fopen(inputFileB, "r");

    if (fileA == NULL || fileB == NULL) {
        perror("Error opening input file");
        fprintf(stderr, "Usage:\n\t.\\matMultp.o a b c\n\t\t  - a : Input matrix name\n\t\t  - b : Input matrix name\n\t\t  - c : Output matrix name\n");
        exit(EXIT_FAILURE);
    }

    // Read matrices from files
    readMatrix(fileA, &A);
    readMatrix(fileB, &B);

    // Close input files
    fclose(fileA);
    fclose(fileB);

    if (A.cols != B.rows){
        fprintf(stderr, "Wrong matrix dimensions! Exiting...\n");
        exit(EXIT_FAILURE);
    }

    // Benchmarking matrix multiplication methods
    benchmark(multiplyMatrix, &matArgs);
    benchmark(multiplyRows, &rowArgs);
    benchmark(multiplyElements, &elemArgs);

    // Write matrices to files
    writeToFile(&matArgs);
    writeToFile(&rowArgs);
    writeToFile(&elemArgs);

    // Display benchmarking results
    printf("Number of threads created per matrix: %d\n", matArgs.threadCount);
    printf("Execution time per matrix: %f seconds\n", matArgs.time);

    printf("---------------------------------------------------------\n");

    printf("Number of threads created per row: %d\n", rowArgs.threadCount);
    printf("Execution time per row: %f seconds\n", rowArgs.time);

    printf("---------------------------------------------------------\n");

    printf("Number of threads created per element: %d\n", elemArgs.threadCount);
    printf("Execution time per element: %f seconds\n", elemArgs.time);

    // Free memory allocated for file names and benchmarking arguments
    free(inputFileA);
    free(inputFileB);
    free(matArgs.outputFile);
    free(rowArgs.outputFile);
    free(elemArgs.outputFile);

    return 0;
}
