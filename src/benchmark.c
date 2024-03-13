#include "benchmark.h"

// Function to perform benchmarking for a matrix multiplication function
void benchmark(void (*multiply)(benchmark_args_t* args), benchmark_args_t* args) {
    // Start the clock
    clock_t start = clock();
    
    // Call the provided matrix multiplication function
    multiply(args);
    
    // End the clock
    clock_t end = clock();
    
    // Calculate the time taken for the multiplication
    args->time = ((double)(end - start)) / CLOCKS_PER_SEC;
}


// Function to write matrix data to a file
void writeToFile(benchmark_args_t* args) {
    FILE* file = fopen(args->outputFile, "w");
    if (file == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Method: A thread per matrix\nrow=%d col=%d\n", args->C->rows, args->C->cols);

    for (int i = 0; i < args->C->rows; i++) {
        for (int j = 0; j < args->C->cols; j++) {
            fprintf(file, "%d ", args->C->mat[i][j]);
        }
        fprintf(file, "\n");
    }

    // Close output files
    fclose(file);
}