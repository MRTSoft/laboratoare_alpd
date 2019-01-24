#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define true 1
#define false 0

int main(int argc, char **args) {
	int SIZE = 1; //Get from MPI
	int coords[2];
	int rank; // rank of the current node
	int Aij, Bij, Cij;

	MPI_Init(&argc, &args);

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int dim = 1;
	while(dim * dim != SIZE && dim * dim < SIZE){
		dim++;
	}
	if (dim * dim != SIZE) {
		if (rank == 0) {
			printf("Number of processes can't be mapped on a square matrix!\n");
		}
		MPI_Finalize();
		return 0;
	}

	int * FM = (int*)malloc(sizeof(int)*SIZE);
	
	//0. Create the topology
	int dims[2] = {dim, dim};
	int periods[2] = {true, true};
	MPI_Comm cartComm;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, false, &cartComm);

	//1. Distribute the elements to the nodes
	srand(0xbeef + rank);

	Aij = (rand() % 89) + 10;
	MPI_Cart_coords(cartComm, rank, 2, coords);
	if (coords[0] == coords[1]){
		Bij = 1;
	} else {
		Bij = 0;
	}
	Cij = 0;
	printf("SRC_%02d Values A: %d | B: %d\n", rank, Aij, Bij);
	
	//2. Initial alignement
	int k;
	int i = coords[0];
	int j = coords[1];
	int nLeft, nRight, nUp, nDown;
	int ncLeft[2] = {i, ((j-1)<0) ? dim-1 : j-1};
	int ncRight[2] = {i, (j+1)%dim};
	int ncUp[2] = {((i-1) < 0)? dim-1: i-1, j};
	int ncDown[2] = {i+1, j};


	MPI_Cart_rank(cartComm, ncLeft, &nLeft);
	MPI_Cart_rank(cartComm, ncRight, &nRight);
	MPI_Cart_rank(cartComm, ncUp, &nUp);
	MPI_Cart_rank(cartComm, ncDown, &nDown);

	printf("Vecini [%2d] %02d*%02d: U-%02d D-%02d L-%02d R-%02d\n", rank, i, j, nUp, nDown, nLeft, nRight); 

	for(k = 1; k <= i; ++k){
		//Trimite Aij catre pi,j-1
		MPI_Send(&Aij, 1, MPI_INT, nLeft, 0xbeef, cartComm);
		MPI_Recv(&Aij, 1, MPI_INT, nRight, 0xbeef, cartComm, NULL);
		//Primeste Aij de la pi,j+1
	}
	for(k = 1; k <= j; ++k){
		MPI_Send(&Bij, 1, MPI_INT, nUp, 0xdead, cartComm);
		MPI_Recv(&Bij, 1, MPI_INT, nDown, 0xdead, cartComm, NULL);
	}
	for(k = 0; k < dim-1; ++k){
		Cij += Aij * Bij;
		MPI_Send(&Aij, 1, MPI_INT, nLeft, 0xbeef, cartComm);
		MPI_Recv(&Aij, 1, MPI_INT, nRight, 0xbeef, cartComm, NULL);
		MPI_Send(&Bij, 1, MPI_INT, nUp, 0xdead, cartComm);
		MPI_Recv(&Bij, 1, MPI_INT, nDown, 0xdead, cartComm, NULL);
	}
	Cij += Aij * Bij;

	//printf("Result [%02d]: %d\n", rank, Cij);
	if (rank == 0){
		FM[0] = Cij;
		for(k = 1; k < SIZE; ++k){
			MPI_Status stats;
			MPI_Recv(&Cij, 1, MPI_INT, MPI_ANY_SOURCE, 0xbeef, MPI_COMM_WORLD, &stats);
			FM[stats.MPI_SOURCE] = Cij;
		}
		for(i=0; i<dim; ++i){
			printf("R%d:\t", i);
			for(j=0; j<dim; ++j){
				printf("%02d ", FM[i*dim+j]);
			}
			printf("\n");
		}
	}
	else {
		MPI_Send(&Cij, 1, MPI_INT, 0, 0xbeef, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}
