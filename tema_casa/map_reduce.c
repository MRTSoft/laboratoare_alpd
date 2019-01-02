#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "master.h"
#include "worker.h"

int main(int argc, char **args) {
	int SIZE = 1; //Get from MPI
	int rank; // rank of the current node
	int exitCode = 0;
	MPI_Init(&argc, &args);

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0){
		exitCode = runMaster("./data-input", "./data-interim", "./data-output");
	}
	else {
		exitCode = runWorker(0);
	}
	MPI_Finalize();
	return exitCode;
}

/*
void relay(int src, int dst, double * buff, int bSize){
	int SIZE = 0, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	MPI_Recv(buff, bSize, MPI_DOUBLE, src, 0xbeef, MPI_COMM_WORLD, NULL);
	printf("relay: %d ---> %d\n", src, dst);
	if (dst < SIZE && dst >= 0)
		MPI_Send(buff, bSize, MPI_DOUBLE, dst, 0xbeef, MPI_COMM_WORLD);
}
*/
