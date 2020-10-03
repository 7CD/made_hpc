#include <stdlib.h>


void zeroArray(double* arr, size_t n) {
    for (size_t i = 0; i < n; ++i) 
        arr[i] = 0.0;
}


void matvec(const double* M, const double* v, double* u, size_t m, size_t n) {
    /*
     * M -- matrix of size m x n
     * v -- vector of size n
     * u -- vector of size m, filled with the result of multiplication M and v
     */
    zeroArray(u, m);

    for(size_t i = 0; i < m; ++i) 
        for(size_t j = 0; j < n; ++j) 
            u[i] += M[i * n + j] * v[j];
}


void matmul(const double* A, const double* B, double* C, size_t m, size_t k, size_t n) {
    /*
     * A -- matrix of size m x k
     * B -- matrix of size k x n
     * C -- matrix of size m x n, filled with the result of multiplication A and B
     */
    zeroArray(C, m * n);
    size_t i, j, t;
    
    for(t = 0; t < k; ++t) 
        for(i = 0; i < m; ++i) 
            for (j = 0; j < n; ++j) 
                C[i * n + j] += A[i * k + t] * B[t * n + j];
}

