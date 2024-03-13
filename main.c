#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 20

// Struct for passing arguments to threads
typedef struct {
    int row;
    int col;
    int (*A)[MAX_SIZE];
    int (*B)[MAX_SIZE];
    int (*C)[MAX_SIZE];
} ThreadArgs;

// Global variables for matrices and their sizes
int rowsA, colsA, rowsB, colsB;
int matrixA[MAX_SIZE][MAX_SIZE];
int matrixB[MAX_SIZE][MAX_SIZE];
int matrixC_per_matrix[MAX_SIZE][MAX_SIZE];
int matrixC_per_row[MAX_SIZE][MAX_SIZE];
int matrixC_per_element[MAX_SIZE][MAX_SIZE];

// Function to read matrix from file
void readMatrix(FILE *file, int matrix[MAX_SIZE][MAX_SIZE], int *rows, int *cols) {
    if (fscanf(file, "row=%d col=%d\n", rows, cols) != 2) {
        perror("Error reading matrix dimensions");
        exit(1);
    }
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *cols; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                perror("Error reading matrix element");
                exit(1);
            }
        }
    }
}

// Function to perform matrix multiplication for a single element
void *multiplyElement(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    int row = threadArgs->row;
    int col = threadArgs->col;
    int result = 0;
    for (int i = 0; i < colsA; i++) {
        result += threadArgs->A[row][i] * threadArgs->B[i][col];
    }
    threadArgs->C[row][col] = result;
    free(threadArgs); // Free threadArgs after usage
    return NULL;
}

// Function to perform matrix multiplication for a row
void *multiplyRow(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    int row = threadArgs->row;
    for (int j = 0; j < colsB; j++) {
        int result = 0;
        for (int k = 0; k < colsA; k++) {
            result += threadArgs->A[row][k] * threadArgs->B[k][j];
        }
        threadArgs->C[row][j] = result;
    }
    free(threadArgs); // Free threadArgs after usage
    return NULL;
}

// Function to perform matrix multiplication for the entire matrix
void multiplyMatrix() {
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            int result = 0;
            for (int k = 0; k < colsA; k++) {
                result += matrixA[i][k] * matrixB[k][j];
            }
            matrixC_per_matrix[i][j] = result;
        }
    }
}

// Function to create threads for matrix multiplication
void createThreads(pthread_t *threads, void *(*threadFunction)(void *), int *threadCount, int (*C)[MAX_SIZE]) {
    for (int i = 0; i < rowsA; i++) {
        ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        args->row = i;
        args->col = 0;
        args->A = matrixA;
        args->B = matrixB;
        args->C = C;
        pthread_create(&threads[*threadCount], NULL, threadFunction, args);
        (*threadCount)++;
    }
}

// Function to create threads for matrix multiplication per element
void createThreadsPerElement(pthread_t *threads, void *(*threadFunction)(void *), int *threadCount, int (*C)[MAX_SIZE]) {
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            args->row = i;
            args->col = j; // Set the column index
            args->A = matrixA;
            args->B = matrixB;
            args->C = C;
            pthread_create(&threads[*threadCount], NULL, threadFunction, args);
            (*threadCount)++;
        }
    }
}


int main(int argc, char *argv[]) {
    // File names
    char *inputFileA = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *inputFileB = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *outputFile_per_matrix = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *outputFile_per_row = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *outputFile_per_element = (char*)malloc(sizeof(char) * MAX_SIZE);

    // Check for custom arguments
    if (argc == 4) {
        strcpy(inputFileA, argv[1]);
        strcat(inputFileA, ".txt");
        strcpy(inputFileB, argv[2]);
        strcat(inputFileB, ".txt");
        strcpy(outputFile_per_matrix, argv[3]);
        strcat(outputFile_per_matrix, "_per_matrix.txt");
        strcpy(outputFile_per_row, argv[3]);
        strcat(outputFile_per_row, "_per_row.txt");
        strcpy(outputFile_per_element, argv[3]);
        strcat(outputFile_per_element, "_per_element.txt");
    }
    else
    {
        strcpy(inputFileA, "a.txt");
        strcpy(inputFileB, "b.txt");
        strcpy(outputFile_per_matrix, "c_per_matrix.txt");
        strcpy(outputFile_per_row, "c_per_row.txt");
        strcpy(outputFile_per_element, "c_per_element.txt");
    }
    

    // Open input files
    FILE *fileA = fopen(inputFileA, "r");
    FILE *fileB = fopen(inputFileB, "r");

    if (fileA == NULL || fileB == NULL) {
        perror("Error opening input file");
        return 1;
    }

    // Read matrices from files
    readMatrix(fileA, matrixA, &rowsA, &colsA);
    readMatrix(fileB, matrixB, &rowsB, &colsB);

    // Close input files
    fclose(fileA);
    fclose(fileB);

    // Initialize threads
    pthread_t threads_per_matrix[MAX_SIZE];
    pthread_t threads_per_row[MAX_SIZE];
    pthread_t threads_per_element[MAX_SIZE];
    int threadCount_per_matrix = 0;
    int threadCount_per_row = 0;
    int threadCount_per_element = 0;

    // Measure execution time for matrix multiplication per matrix
    clock_t start_per_matrix = clock();
    multiplyMatrix();
    clock_t end_per_matrix = clock();
    double time_spent_per_matrix = ((double)(end_per_matrix - start_per_matrix)) / CLOCKS_PER_SEC;

    // Measure execution time for matrix multiplication per row
    clock_t start_per_row = clock();
    createThreads(threads_per_row, multiplyRow, &threadCount_per_row, matrixC_per_row);
    for (int i = 0; i < threadCount_per_row; i++) {
        pthread_join(threads_per_row[i], NULL);
    }
    clock_t end_per_row = clock();
    double time_spent_per_row = ((double)(end_per_row - start_per_row)) / CLOCKS_PER_SEC;

    // Measure execution time for matrix multiplication per element
    clock_t start_per_element = clock();
    createThreads(threads_per_element, multiplyElement, &threadCount_per_element, matrixC_per_element);
    for (int i = 0; i < threadCount_per_element; i++) {
        pthread_join(threads_per_element[i], NULL);
    }
    clock_t end_per_element = clock();
    double time_spent_per_element = ((double)(end_per_element - start_per_element)) / CLOCKS_PER_SEC;

    // Output matrices to files
    FILE *file_per_matrix = fopen(outputFile_per_matrix, "w");
    FILE *file_per_row = fopen(outputFile_per_row, "w");
    FILE *file_per_element = fopen(outputFile_per_element, "w");

    if (file_per_matrix == NULL || file_per_row == NULL || file_per_element == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Write matrices to files
    fprintf(file_per_matrix, "Method: A thread per matrix\n");
    fprintf(file_per_row, "Method: A thread per row\n");
    fprintf(file_per_element, "Method: A thread per element\n");

    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            fprintf(file_per_matrix, "%d ", matrixC_per_matrix[i][j]);
            fprintf(file_per_row, "%d ", matrixC_per_row[i][j]);
            fprintf(file_per_element, "%d ", matrixC_per_element[i][j]);
        }
        fprintf(file_per_matrix, "\n");
        fprintf(file_per_row, "\n");
        fprintf(file_per_element, "\n");
    }

    // Close output files
    fclose(file_per_matrix);
    fclose(file_per_row);
    fclose(file_per_element);

    // Print execution times
    printf("Execution time per matrix: %f seconds\n", time_spent_per_matrix);
    printf("Execution time per row: %f seconds\n", time_spent_per_row);
    printf("Execution time per element: %f seconds\n", time_spent_per_element);

    return 0;
}
