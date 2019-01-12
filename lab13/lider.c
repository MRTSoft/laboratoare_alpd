#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **args) {
	int SIZE = 1; //Get from MPI
	int rank; // rank of the current node
	int lider = -1;
	int aux;
    
	int index[] = {
		4,
		6,
		9,
		12,
		15,
		18,
		21,
		25,
		28
	};
	int edges[] = {
		1,4,5,6,
		0,6,
		4,7,8,
		5,6,8,
		0,2,7,
		0,3,7,
		0,1,3,
		2,4,5,8,
		2,3,7
	};

	int neighbors[9];
	int nCount = -1;

	MPI_Init(&argc, &args);

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm commGraph;
	MPI_Graph_create(MPI_COMM_WORLD, 9, index, edges, 0, &commGraph);
	MPI_Graph_neighbors_count(commGraph, rank, &nCount);
	MPI_Graph_neighbors(commGraph, rank, 9, neighbors);


	lider = rank;
	int d = 3;//TODO: Read din args

	int round;
	int i;
	for(round = 0; round < d; ++round){
		// send to all neighbors
		for(i=0; i < nCount; ++i){
			MPI_Send(&lider, 1, MPI_INT, neighbors[i], 0x7A6, MPI_COMM_WORLD);
		}
		for(i = 0; i < nCount; ++i){
			MPI_Recv(&aux, 1, MPI_INT, neighbors[i], 0x7A6, MPI_COMM_WORLD, NULL);
			if (aux > lider) {
				lider = aux;
			}
		}
		// recv from all neighbors
		// keep highest leader
	}
      	//MPI_Send(&Aij, 1, MPI_INT, nLeft, 0xbeef, MPI_COMM_WORLD);
	//MPI_Recv(&Aij, 1, MPI_INT, nRight, 0xbeef, MPI_COMM_WORLD, NULL);
	printf("[%02d]The leader is [%02d] \n", rank, lider);

	MPI_Finalize();
	return 0;
}
