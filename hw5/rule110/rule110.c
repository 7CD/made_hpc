#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>


// indexes from 000 to 111
const int rule110[] = {0, 1, 1, 1, 0, 1, 1, 0};
const int rule30[] = {0, 1, 1, 1, 1, 0, 0, 0};

#define N_CELLS 16
#define N_STEPS 10
#define PERIODIC_BOUNDARY 1
#define RULE rule110

void rule(int* u0, int* u1, const int* xrule);
void init(int* u0, int N, int prank);
void exchange(int* u0, int prank, int psize, int pncells);
void collect(int* uglobal, int* u0, int prank, int psize, int pncells);
void print(int* a, int n);


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    
    int psize, prank;
    MPI_Comm_size(MPI_COMM_WORLD, &psize);
	MPI_Comm_rank(MPI_COMM_WORLD, &prank);
	
	assert(psize > 1);
	
	// поле на текущем шаге
	int* u0;
	// поле на следующем шаге
	int* u1;
	
	int pncells = N_CELLS / psize;
	
	if (prank == psize - 1)
	    pncells += N_CELLS % psize;
	
	// 2 -- для клеток-призраков на концах массивов
	u0 = (int*)malloc((pncells + 2) * sizeof(int));
	u1 = (int*)malloc((pncells + 2) * sizeof(int));

	init(u0, pncells + 2, prank);
	u1[0] = 0; u1[pncells + 1] = 0;
	
	int* uglobal;
	if (prank == 0)
	    uglobal = (int*)calloc(N_CELLS, sizeof(int));

	collect(uglobal, u0, prank, psize, pncells);
	
	if (prank == 0) {
	    print(uglobal, N_CELLS);
	}
	
	for (int i = 1; i <= N_STEPS; ++i) {
	    
	    exchange(u0, prank, psize, pncells);
	    
	    // step next
	    for (int c = 1; c < pncells + 1; ++c) {
	        rule(&u0[c], &u1[c], RULE);
	    }
	    
    	int* tmp = u0;
        u0 = u1;
        u1 = tmp;

	    collect(uglobal, u0, prank, psize, pncells);
	    
	    if (prank == 0) {
	        print(uglobal, N_CELLS);
	    }
	}
	
    if (prank == 0)
        free(uglobal);
    free(u0);
    free(u1);
    MPI_Finalize();

    return 0;
}


void rule(int* u0, int* u1, const int* xrule) {

    int bcode = *(u0 + 1) + 2 * (*u0) + 4 * (*(u0 - 1));
    
    *u1 = xrule[bcode];
}


void init(int* u0, int N, int prank) {
    srand(time(0) + prank);
    u0[0] = u0[N - 1] = 0;
    for (size_t i = 1; i < N - 1; i++)
        u0[i] = rand() % 2;
}


void exchange(int* u0, int prank, int psize, int pncells) {
    // exchange as if PERIODIC_BOUNDARY
    int pleft = (prank - 1 + psize) % psize; 
    int pright = (prank + 1 + psize) % psize;
    
    MPI_Request rq[4];
    MPI_Irecv(u0, 1, MPI_INT, pleft, 1, MPI_COMM_WORLD, rq+0);
	MPI_Isend(u0 + 1, 1, MPI_INT, pleft, 1, MPI_COMM_WORLD, rq+1);
	MPI_Isend(u0 + pncells, 1, MPI_INT, pright, 1, MPI_COMM_WORLD, rq+2);
	MPI_Irecv(u0 + pncells + 1, 1, MPI_INT, pright, 1, MPI_COMM_WORLD, rq+3);
	MPI_Waitall(4, rq, MPI_STATUSES_IGNORE);
    
    if (!PERIODIC_BOUNDARY) {
        if (prank == 0) 
            u0[0] = 0;
        else if (prank == psize - 1)
            u0[pncells + 1] = 0;
    }
}


void collect(int* uglobal, int* u0, int prank, int psize, int pncells) {
    if (prank == 0) {
        memcpy(uglobal, u0 + 1, pncells * sizeof(int));
        for (int pfrom = 1; pfrom < psize - 1; ++pfrom) 
            MPI_Recv(uglobal + pfrom * pncells, pncells, MPI_INT, pfrom, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        MPI_Recv(uglobal + (psize - 1) * pncells, pncells + N_CELLS % psize, MPI_INT, psize - 1, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
    } else {
        MPI_Send(u0 + 1, pncells, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } 
}


void print(int* a, int n) {
	for (int i = 0; i < n; ++i) 
	    printf("%d ", a[i]);
	printf("\n");
}
