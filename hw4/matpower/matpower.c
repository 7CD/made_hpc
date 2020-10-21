#include <stdlib.h>
#include <string.h>


void zeroArray(int* arr, size_t n) {
    for (size_t i = 0; i < n; ++i) 
        arr[i] = 0;
}


void matmul(int* A, int* B, int* C, size_t N) {
    zeroArray(C, N * N);
    size_t i, j, k;
    
    for(k = 0; k < N; k++)
        for(i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}


void _matpower_recursion(int* A, int* T, int* result, size_t N, size_t power) {
    if (power == 2) {
        matmul(A, A, result, N);
        return;
    } else if (N % 2 == 1) {
        _matpower_recursion(A, result, T, N, power - 1);
        matmul(T, A, result, N);
    } else {
        _matpower_recursion(A, result, T, N, power / 2);
        matmul(T, T, result, N);
    }
}


void matpower(int* A, int* result, size_t N, size_t power) {

    if (power == 0) {
        zeroArray(result, N * N);
        for (int i = 0; i < N; ++i)
            result[i * N + i] = 1;
        return;
    } else if (power == 1) {
        memcpy(result, A, N * N * sizeof(int));
        return;
    }
    
    int* T = (int*) malloc(N * N * sizeof(int));
    
    _matpower_recursion(A, T, result, N, power);
        
    free(T);
}	

