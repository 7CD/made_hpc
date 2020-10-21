#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "header.h"


void print_matrix(int* C, int N) {
    printf("\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            printf("%d ", C[i * N + j]);
        printf("\n");
    }
    printf("\n");
}


void assert_equals(int* A, int* B, size_t N) {
    for (int i = 0; i < N * N; ++i)
        assert(A[i] == B[i]);
}


int main() {
    int A[] = {0, 1, 0, 0, 0,
	           0, 0, 1, 0, 0,
               0, 0, 0, 1, 0,
	           0, 0, 0, 0, 1,
	           0, 1, 1, 0, 0};
	
    int N = 5;
    int* result = (int*) malloc(N * N * sizeof(int));
    
    int m = 2;
    matpower(A, result, N, m);
    int A_2[] = {0, 0, 1, 0, 0,
	             0, 0, 0, 1, 0,
                 0, 0, 0, 0, 1,
	             0, 1, 1, 0, 0,
	             0, 0, 1, 1, 0};
	assert_equals(result, A_2, N);
	
	m = 6;             
	matpower(A, result, N, m);
    int A_6[] = {0, 0, 1, 1, 0,
	             0, 0, 0, 1, 1,
                 0, 1, 1, 0, 1,
	             0, 1, 2, 1, 0,
	             0, 0, 1, 2, 1};
	assert_equals(result, A_6, N);
	
	printf("Tests passed\n");
    
    return 0;
}
