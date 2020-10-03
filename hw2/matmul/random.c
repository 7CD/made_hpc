#include <sys/time.h>
#include <stdlib.h>

void randomArray(double* arr, size_t N) {
    srand(time(NULL));

    for (size_t i = 0; i < N; i++)
        arr[i] = rand() / RAND_MAX;
}
