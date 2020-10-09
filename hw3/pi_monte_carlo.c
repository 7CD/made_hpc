#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("pass two arguments: num_threads, n_points\n");
        return 0;
    }
    omp_set_num_threads(atoi(argv[1]));
    int num_points = atoi(argv[2]);
   
    double start_t, end_t;
    
    start_t = omp_get_wtime();
    
// Используем единичный квадрат и вписанную в него четверть круга
    int points_in_circle_total = 0;

#pragma omp parallel default(none) shared(num_points, points_in_circle_total) 
    {
        int tid = omp_get_thread_num();
        
        struct timeval time_;
        gettimeofday(&time_, NULL);
        unsigned int seed = (time_.tv_usec * 10 + tid) * 100 + time_.tv_usec / (tid + 1);
  
        
        double x, y;
        int points_in_circle;
        #pragma omp for
        for (int i = 0; i < num_points; ++i) {
             x = (double)rand_r(&seed) / RAND_MAX;
             y = (double)rand_r(&seed) / RAND_MAX;
             if (x * x + y * y < 1) 
                 ++points_in_circle;
        }
        
        #pragma omp atomic 
        points_in_circle_total += points_in_circle;
    }
    
    // S_circle / S_rectangle = points_in_circle_total / num_points
    // S_circle = 1 / 4 pi r^2
    double pi = 4 * (double) points_in_circle_total / num_points;
    
    end_t = omp_get_wtime();
    
    printf("pi ~= % lf\n", pi);
    printf("Time elapsed: %lf sec\n", end_t  - start_t);

    return 0;
}

