#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <mpi.h>

// i-й столбец, j-я строка, с периодическими граничными условиями
#define ind(i, j) (i + l->nx) % l->nx + ((j + l->ny) % l->ny) * l->nx


typedef struct {
	int nx; // число столбцов поля
	int ny; // число строк поля
	// буфферы для поля на текущем и следующем шаге
	int* u0;
	int* u1; 
	int steps;
	int save_steps;
    
    int psize, prank;
	int start_col, end_col;
} life_t;


void life_init(const char *path, life_t *l);
void decomposition(const int prank, const int psize, const int nx, int* start_col, int* end_col);
void life_step(life_t* l);
void life_collect(life_t* l);
void life_exchange(life_t* l);
void life_free(life_t* l);
void life_save_vtk(const char* path, life_t* l);


MPI_Datatype VERTICAL_BLOCK;
MPI_Datatype LAST_VERTICAL_BLOCK;
MPI_Datatype COLUMN;


int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s input file.\n", argv[0]);
		return 0;
	}
	
    MPI_Init(&argc, &argv);
	
	life_t l;
	life_init(argv[1], &l);	

	MPI_Type_vector (l.ny, l.nx / l.psize, l.nx, MPI_INT, &VERTICAL_BLOCK);
	MPI_Type_commit (&VERTICAL_BLOCK);

	MPI_Type_vector (l.ny, l.nx / l.psize + l.nx % l.psize, l.nx, MPI_INT, &LAST_VERTICAL_BLOCK);
	MPI_Type_commit (&LAST_VERTICAL_BLOCK);
	
    MPI_Type_vector (l.ny, 1, l.nx, MPI_INT, &COLUMN);
	MPI_Type_commit (&COLUMN); 

	char buf[100];
	for (int i = 0; i < l.steps; ++i) {
		if (i % l.save_steps == 0) {
			life_collect(&l); // 
			sprintf(buf, "life_%06d.vtk", i);
			printf("Saving step %d to '%s'.\n", i, buf);
			if (l.prank == 0) 
				life_save_vtk(buf, &l);
		}	
		life_step(&l); //
	}
	
	MPI_Type_free (&COLUMN);
	MPI_Type_free (&VERTICAL_BLOCK);
	MPI_Type_free (&LAST_VERTICAL_BLOCK);
	
    MPI_Finalize();
	life_free(&l);
	return 0;
}


/*
 * Загрузить входную конфигурацию из файла.
 * Формат файла: число шагов, как часто сохранять, размер поля, затем идут координаты заполненных клеток:
 * steps
 * save_steps
 * nx ny
 * i1 j2
 * i2 j2
 */
void life_init(const char *path, life_t *l) {
	FILE *fd = fopen(path, "r");
	assert(fd);
	assert(fscanf(fd, "%d\n", &l->steps));
	assert(fscanf(fd, "%d\n", &l->save_steps));
	printf("Steps %d, save every %d step.\n", l->steps, l->save_steps);
	assert(fscanf(fd, "%d %d\n", &l->nx, &l->ny));
	printf("Field size: %dx%d\n", l->nx, l->ny);

	l->u0 = (int*)calloc(l->nx * l->ny, sizeof(int));
	l->u1 = (int*)calloc(l->nx * l->ny, sizeof(int));
	
	int i, j, r, cnt;
	cnt = 0;
	while ((r = fscanf(fd, "%d %d\n", &i, &j)) != EOF) {
		l->u0[ind(i, j)] = 1;
		cnt++;
	}
	printf("Loaded %d life cells.\n", cnt);
	fclose(fd);

	MPI_Comm_size(MPI_COMM_WORLD, &(l->psize));
	MPI_Comm_rank(MPI_COMM_WORLD, &(l->prank));
	decomposition(l->prank, l->psize, l->nx, &(l->start_col), &(l->end_col));
}


void decomposition(const int prank, const int psize, const int nx, int* start_col, int* end_col) {
	int columns_per_process = nx / psize;
	*start_col = columns_per_process * prank;
	*end_col = *start_col + columns_per_process;
	// последний процесс получает остаток колонок
	if (prank == psize - 1) 
	    *end_col = nx;
}


// все блоки пересылаются нулевому процессу
void life_collect (life_t *l) {
	if (l->prank == 0) {
	    int start_col, end_col;
		for (int i = 1; i < l->psize - 1; i++) {
			decomposition(i, l->psize, l->nx, &start_col, &end_col);
			MPI_Recv(l->u0 + ind(start_col, 0), 1, VERTICAL_BLOCK, i, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		}
		decomposition(l->psize - 1, l->psize, l->nx, &start_col, &end_col);
		MPI_Recv(l->u0 + ind(start_col, 0), 1, LAST_VERTICAL_BLOCK, l->psize - 1, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
	}
	else if (l->prank == l->psize - 1) {
		MPI_Send(l->u0+ind(l->start_col, 0), 1, LAST_VERTICAL_BLOCK, 0, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Send(l->u0+ind(l->start_col, 0), 1, VERTICAL_BLOCK, 0, 0, MPI_COMM_WORLD);
	}	
}


// процессы обменниваются пограничными колонками
void life_exchange (life_t* l) {
	#define indp(p) (((p) + l->psize) % l->psize)
	MPI_Request rq[4];
	MPI_Irecv(l->u0 + ind(l->start_col-1, 0), 1, COLUMN, indp(l->prank-1), 1, MPI_COMM_WORLD, rq+0);
	MPI_Isend(l->u0 + ind(l->start_col, 0), 1, COLUMN, indp(l->prank-1), 1, MPI_COMM_WORLD, rq+1);
	MPI_Isend(l->u0 + ind(l->end_col-1, 0), 1, COLUMN, indp(l->prank+1), 1, MPI_COMM_WORLD, rq+2);
	MPI_Irecv(l->u0 + ind(l->end_col, 0), 1, COLUMN, indp(l->prank+1), 1, MPI_COMM_WORLD, rq+3);
	MPI_Waitall(4, rq, MPI_STATUSES_IGNORE);
	#undef indp	
}


void life_step(life_t* l) {
	life_exchange(l);

	int i, j;
	for (j = 0; j < l->ny; j++) {
		for (i = l->start_col; i < l->end_col; i++) {
			int n = 0; // число соседей клетки
			n += l->u0[ind(i+1, j)];
			n += l->u0[ind(i+1, j+1)];
			n += l->u0[ind(i,   j+1)];
			n += l->u0[ind(i-1, j)];
			n += l->u0[ind(i-1, j-1)];
			n += l->u0[ind(i,   j-1)];
			n += l->u0[ind(i-1, j+1)];
			n += l->u0[ind(i+1, j-1)];
			l->u1[ind(i,j)] = 0;
			if (n == 3 && l->u0[ind(i,j)] == 0) {
				l->u1[ind(i,j)] = 1;
			}
			if ((n == 3 || n == 2) && l->u0[ind(i,j)] == 1) {
				l->u1[ind(i,j)] = 1;
			}
		}
	}
	int *tmp;
	tmp = l->u0;
	l->u0 = l->u1;
	l->u1 = tmp;
}


void life_free(life_t* l) {
	free(l->u0);
	free(l->u1);
	l->nx = l->ny = 0;
}


void life_save_vtk(const char* path, life_t* l) {
	FILE *f;
	int i1, i2, j;
	f = fopen(path, "w");
	assert(f);
	fprintf(f, "# vtk DataFile Version 3.0\n");
	fprintf(f, "Created by write_to_vtk2d\n");
	fprintf(f, "ASCII\n");
	fprintf(f, "DATASET STRUCTURED_POINTS\n");
	fprintf(f, "DIMENSIONS %d %d 1\n", l->nx+1, l->ny+1);
	fprintf(f, "SPACING %d %d 0.0\n", 1, 1);
	fprintf(f, "ORIGIN %d %d 0.0\n", 0, 0);
	fprintf(f, "CELL_DATA %d\n", l->nx * l->ny);
	
	fprintf(f, "SCALARS life int 1\n");
	fprintf(f, "LOOKUP_TABLE life_table\n");
	for (i2 = 0; i2 < l->ny; i2++) {
		for (i1 = 0; i1 < l->nx; i1++) {
			fprintf(f, "%d\n", l->u0[ind(i1, i2)]);
		}
	}
	fclose(f);
}

