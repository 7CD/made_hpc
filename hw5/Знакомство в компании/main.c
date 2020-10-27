#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void init_state(int* state, int psize) {
    int* done_processes = state;
    int* remaining_processes = state + psize;
    state[2 * psize] = 0; // n_done
    state[2 * psize + 1] = psize; // n_remaining

    for (int i = 0; i < psize; ++i) 
        remaining_processes[i] = i;
}


int state_next(int* state, int prank, int psize) {
    int* done_processes = state;
    int* remaining_processes = state + psize;
    int n_done = state[2 * psize];
    int n_remaining = state[2 * psize + 1];
    
    done_processes[n_done] = prank;
    state[2 * psize] = ++n_done;
    
    
    // find and remove prank from remaining
    int i;
    for (i = 0; i < n_remaining; ++i)
        if (prank == remaining_processes[i])
            break;
    remaining_processes[i] = remaining_processes[n_remaining - 1];
    state[2 * psize + 1] = --n_remaining;
    
    int send_to_rank = -1;
    if (n_remaining > 0) 
        send_to_rank = remaining_processes[rand() % n_remaining];
    
    return send_to_rank;
}


void print_array(int* arr, int N) {
    for (int i = 0; i < N; ++i)
        printf("%d ", arr[i]);
    printf("\n");
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int psize;
    MPI_Comm_size(MPI_COMM_WORLD, &psize);
    int prank;
    MPI_Comm_rank(MPI_COMM_WORLD, &prank);
    
    assert(psize > 1);
    
    srand(time(NULL));
    
    /*
      В state последовательно храним массив обработанных процессов,
      массив необработанных процессов,
      число обработанных поцессов,
      число необработанных процессов
    */
    int* state = (int*)calloc(2 * psize + 2, sizeof(int));
    init_state(state, psize);
   
    if (prank == 0) {
    
        int send_to = state_next(state, prank, psize);
        
        MPI_Ssend(state, 2 * psize + 2, MPI_INT, send_to, 99, MPI_COMM_WORLD);
 
    } else {
    
        MPI_Recv(state, 2 * psize + 2, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int send_to = state_next(state, prank, psize);

        if (send_to > 0) 
            MPI_Ssend(state, 2 * psize + 2, MPI_INT, send_to, 99, MPI_COMM_WORLD);
        else 
            print_array(state, psize); // Выводим процессы по порядку
    }
    
    free(state);
    MPI_Finalize();
    
    return 0;
}
