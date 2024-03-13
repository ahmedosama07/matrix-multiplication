#include "matrix.h"

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
    return NULL;
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
    data->C->mat[row][col] = result;
    free(data);
    return NULL;
}

// Function to perform matrix multiplication for the entire matrix using threads
void multiplyMatrix(benchmark_args_t* bench_args) {
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

// Function to perform matrix multiplication for each row using threads
void multiplyRows(benchmark_args_t* bench_args) {
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

// Function to perform matrix multiplication for each element using threads
void multiplyElements(benchmark_args_t* bench_args) {
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
