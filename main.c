#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE 20

typedef struct 
{
    int rows;
    int cols;
    int** mat;
} matrix_t;

typedef struct 
{
    int currentRow;
    int currentCol;
    matrix_t* A;
    matrix_t* B;
    matrix_t* C;
} thread_args_t;



// Function to read matrix from file
void readMatrix(FILE *file, matrix_t* mat) {
    if (fscanf(file, "row=%d col=%d\n", &(mat->rows), &(mat->cols)) != 2) {
        perror("Error reading matrix dimensions");
        exit(1);
    }
    mat->mat = (int**)malloc(sizeof(int*) * mat->rows);

    for (int i = 0; i < mat->rows; i++) {
        mat->mat[i] = (int*)malloc(sizeof(int) * mat->cols);
        for (int j = 0; j < mat->cols; j++) {
            if (fscanf(file, "%d", &(mat->mat[i][j])) != 1) {
                perror("Error reading matrix element");
                exit(1);
            }
        }
    }
}

// Function to perform matrix multiplication for the entire matrix
void *multiplyMatrix(void *args) {
    thread_args_t *data = (thread_args_t *)args;
    data->C->rows = data->A->rows;
    data->C->cols = data->B->cols;
    data->C->mat = (int**)malloc(sizeof(int*) * data->C->rows);
    for (int i = 0; i < data->A->rows; i++) {
        data->C->mat[i] = (int*)malloc(sizeof(int) * data->C->cols);
        for (int j = 0; j < data->B->cols; j++) {
            int result = 0;
            for (int k = 0; k < data->A->cols; k++) {
                result += data->A->mat[i][k] * data->B->mat[k][j];
            }
            data->C->mat[i][j] = result;
        }
    }
}

// Function to perform matrix multiplication for a row
void *multiplyRow(void *args) {
    thread_args_t *data = (thread_args_t *)args;
    int row = data->currentRow;
    for (int j = 0; j < data->B->cols; j++) {
        int result = 0;
        for (int k = 0; k < data->A->cols; k++) {
            result += data->A->mat[row][k] * data->B->mat[k][j];
        }
        data->C->mat[row][j] = result;
    }
    free(data);
    return NULL;
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

    matrix_t A;
    matrix_t B;
    matrix_t C;

    // Read matrices from files
    readMatrix(fileA, &A);
    readMatrix(fileB, &B);

    // Close input files
    fclose(fileA);
    fclose(fileB);


    // Measure execution time for matrix multiplication per matrix
    clock_t start_per_matrix = clock();
    pthread_t matrixThread;
    thread_args_t *args = (thread_args_t*)malloc(sizeof(thread_args_t));
    args->A = &A;
    args->B = &B;
    args->C = &C;
    if (pthread_create(&matrixThread, NULL, multiplyMatrix, args) != 0)
    {
        perror("Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(matrixThread , NULL);
    free(args);
    clock_t end_per_matrix = clock();
    double time_spent_per_matrix = ((double)(end_per_matrix - start_per_matrix)) / CLOCKS_PER_SEC;

    // Output matrices to files
    FILE *file_per_matrix = fopen(outputFile_per_matrix, "w");
    
    if (file_per_matrix == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Write matrices to files
    fprintf(file_per_matrix, "Method: A thread per matrix\nrow=%d col=%d\n", C.rows, C.cols);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            fprintf(file_per_matrix, "%d ", C.mat[i][j]);
        }
        fprintf(file_per_matrix, "\n");
    }

    // Close output files
    fclose(file_per_matrix);

    printf("Number of threads created per matrix: %d\n", 1);


    // Print execution times
    printf("Execution time per matrix: %f seconds\n", time_spent_per_matrix);
    
    return 0;
}
