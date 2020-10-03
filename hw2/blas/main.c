#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cblas.h>


double calcMatMulTime(double* A, double* B, double* C, int N, int nRuns) {
    double t_sum = 0;
    struct timeval start, end;

    for (int i = 0; i < nRuns; ++i) {
        gettimeofday(&start, NULL);
	//
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, 1.0, A, N, B, N, 0.0, C, N);
	//
        gettimeofday(&end, NULL);
        double time = end.tv_sec - start.tv_sec + ((double) (end.tv_usec - start.tv_usec)) / 1000000;
        t_sum += time;
    }

    return t_sum / nRuns;
}


void randomArray(double* arr, size_t N) {
    srand(time(NULL));
    for (size_t i = 0; i < N; i++)
        arr[i] = rand() / RAND_MAX;
}


int main() {
    int nRuns = 5;

    double* A = (double *) malloc(4096 * 4096 * sizeof(double));
    double* B = (double *) malloc(4096 * 4096 * sizeof(double));
    double* C = (double *) malloc(4096 * 4096 * sizeof(double));
    
    randomArray(A, 4096 * 4096);
    randomArray(B, 4096 * 4096);

    int Ns[] = {512, 1024, 2048, 4096};

    FILE* fp= fopen("./runtimes.txt", "w+");
    
    for (int i = 0; i < 4; ++i) {
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

