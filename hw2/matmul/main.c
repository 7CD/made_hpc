#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "header.h"


double calcMatMulTime(double* A, double* B, double* C, int N, int nRuns) {
    double t_sum = 0;
    struct timeval start, end;

    for (int i = 0; i < nRuns; ++i) {
        gettimeofday(&start, NULL);
        matmul(A, B, C, N, N, N);
        gettimeofday(&end, NULL);
        double time = end.tv_sec - start.tv_sec + ((double) (end.tv_usec - start.tv_usec)) / 1000000;
        t_sum += time;
    }

    return t_sum / nRuns;
}


int main() {
    int nRuns = 5;

    double* A = (double *) malloc(4096 * 4096 * sizeof(double));
    double* B = (double *) malloc(4096 * 4096 * sizeof(double));
    double* C = (double *) malloc(4096 * 4096 * sizeof(double));
    
    randomArray(A, 4096 * 4096);
    randomArray(B, 4096 * 4096);

    int Ns[] = {512, 1024, 2048};

    FILE* fp= fopen("./runtimes.txt", "w+");
    
    for (int i = 0; i < 3; ++i) {
        int N = Ns[i];
        double time = calcMatMulTime(A, B, C, N, nRuns);
        fprintf(fp, "runtime of %dx%d matmul: %lf seconds\n", N, N, time);
    }
    
    fclose(fp);
    free(A);
    free(B);
    free(C); 

    return 0;
}

