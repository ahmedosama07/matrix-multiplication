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

typedef struct
{
    char* outputFile;
    int threadCount;
    double time;
    matrix_t* A;
    matrix_t* B;
    matrix_t* C;
} benchmark_args_t;


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
void *multiplyMatrixThread(void *args) {
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
void *multiplyRowThread(void *args) {
    thread_args_t *data = (thread_args_t *)args;
    int row = data->currentRow;
    data->C->mat[row] = (int*)malloc(sizeof(int) * data->C->cols);

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

// Function to perform matrix multiplication for a single element
void *multiplyElementThread(void *args) {
    thread_args_t* data = (thread_args_t*)args;
    int row = data->currentRow;
    int col = data->currentCol;
    
    int result = 0;
    for (int i = 0; i < data->A->cols; i++) {
        result += data->A->mat[row][i] * data->B->mat[i][col];
    }
    //printf("%d\n", result);
    data->C->mat[row][col] = result;
    free(data);
    return NULL;
}

void benchmark(int (*multiply)(benchmark_args_t* args), benchmark_args_t* args) {
    clock_t start = clock();
    multiply(args);
    clock_t end = clock();
    args->time = ((double)(end - start)) / CLOCKS_PER_SEC;
}

int multiplyMatrix(benchmark_args_t* bench_args) {
    pthread_t matrixThread;
    thread_args_t *args = (thread_args_t*)malloc(sizeof(thread_args_t));
    args->A = bench_args->A;
    args->B = bench_args->B;
    args->C = bench_args->C;
    if (pthread_create(&matrixThread, NULL, multiplyMatrixThread, args) != 0)
    {
        perror("Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(matrixThread , NULL);
    free(args);
    bench_args->threadCount = 1;
}


int multiplyRows(benchmark_args_t* bench_args) {
    bench_args->C->rows = bench_args->A->rows;
    bench_args->C->cols = bench_args->B->cols;
    bench_args->C->mat = (int**)malloc(sizeof(int*) * bench_args->C->rows);
    
    pthread_t threads_per_row[bench_args->C->rows];
    bench_args->threadCount = 0; 
    
    for (int i = 0; i < bench_args->C->rows; i++) {
        thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t));
        args->currentRow = i;
        args->currentCol = 0;
        args->A = bench_args->A;
        args->B = bench_args->B;
        args->C = bench_args->C;
        
        if (pthread_create(&threads_per_row[bench_args->threadCount], NULL, multiplyRowThread, args) != 0)
        {
            perror("Error creating thread\n");
            exit(EXIT_FAILURE);
        }
        bench_args->threadCount++;
    }
    for (int i = 0; i < bench_args->threadCount; i++) {
        pthread_join(threads_per_row[i], NULL);
    }
}

int multiplyElements(benchmark_args_t* bench_args) {
    bench_args->C->rows = bench_args->A->rows;
    bench_args->C->cols = bench_args->B->cols;
    bench_args->C->mat = (int**)malloc(sizeof(int*) * bench_args->C->rows);
    for (int i = 0; i < bench_args->C->rows; i++) {
        bench_args->C->mat[i] = (int*)malloc(sizeof(int) * bench_args->C->cols);
    }
    
    pthread_t threads_per_row[(bench_args->C->rows) * (bench_args->C->cols)];
    bench_args->threadCount = 0; 
    
    for (int i = 0; i < bench_args->C->rows; i++) {
        for (int j = 0; j < bench_args->C->cols; j++) {
            thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t));
            args->currentRow = i;
            args->currentCol = j;
            args->A = bench_args->A;
            args->B = bench_args->B;
            args->C = bench_args->C;
            
            if (pthread_create(&threads_per_row[bench_args->threadCount], NULL, multiplyElementThread, args) != 0)
            {
                perror("Error creating thread\n");
                exit(EXIT_FAILURE);
            }
            bench_args->threadCount++;
        }
    }
    for (int i = 0; i < bench_args->threadCount; i++) {
        pthread_join(threads_per_row[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    // File names
    char *inputFileA = (char*)malloc(sizeof(char) * MAX_SIZE);
    char *inputFileB = (char*)malloc(sizeof(char) * MAX_SIZE);
    matrix_t A;
    matrix_t B;

    matrix_t C_mat;
    benchmark_args_t matArgs;
    matArgs.A = &A;
    matArgs.B = &B;
    matArgs.C = &C_mat;
    matArgs.outputFile = (char*)malloc(sizeof(char) * MAX_SIZE);

    matrix_t C_row;
    benchmark_args_t rowArgs;
    rowArgs.A = &A;
    rowArgs.B = &B;
    rowArgs.C = &C_row;
    rowArgs.outputFile = (char*)malloc(sizeof(char) * MAX_SIZE);

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
        return 1;
    }

    // Read matrices from files
    readMatrix(fileA, &A);
    readMatrix(fileB, &B);

    // Close input files
    fclose(fileA);
    fclose(fileB);

    benchmark(multiplyMatrix, &matArgs);
    benchmark(multiplyRows, &rowArgs);
    benchmark(multiplyElements, &elemArgs);

    FILE *file_per_matrix = fopen(matArgs.outputFile, "w");
    FILE *file_per_row = fopen(rowArgs.outputFile, "w");
    FILE *file_per_element = fopen(elemArgs.outputFile, "w");
    
    if (file_per_matrix == NULL || file_per_row == NULL || file_per_element == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Write matrices to files
    fprintf(file_per_matrix, "Method: A thread per matrix\nrow=%d col=%d\n", C_mat.rows, C_mat.cols);
    fprintf(file_per_row, "Method: A thread per row\nrow=%d col=%d\n", C_row.rows, C_row.cols);
    fprintf(file_per_element, "Method: A thread per element\nrow=%d col=%d\n", C_elem.rows, C_elem.cols);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            fprintf(file_per_matrix, "%d ", C_mat.mat[i][j]);
            fprintf(file_per_row, "%d ", C_row.mat[i][j]);
            fprintf(file_per_element, "%d ", C_elem.mat[i][j]);
        }
        fprintf(file_per_matrix, "\n");
        fprintf(file_per_row, "\n");
        fprintf(file_per_element, "\n");
    }

    // Close output files
    fclose(file_per_matrix);
    fclose(file_per_row);
    fclose(file_per_element);

    printf("Number of threads created per matrix: %d\n", matArgs.threadCount);
    printf("Execution time per matrix: %f seconds\n", matArgs.time);

    printf("---------------------------------------------------------\n");

    printf("Number of threads created per row: %d\n", rowArgs.threadCount);
    printf("Execution time per row: %f seconds\n", rowArgs.time);

    printf("---------------------------------------------------------\n");

    printf("Number of threads created per element: %d\n", elemArgs.threadCount);
    printf("Execution time per element: %f seconds\n", elemArgs.time);

    return 0;
}
