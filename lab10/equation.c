#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

void relay(int src, int dst, double * buff, int bSize);

int main(int argc, char **args) {
	int SIZE = 1; //Get from MPI
	int rank; // rank of the current node
	double * A;
	double * buff;

	MPI_Init(&argc, &args);

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//0. Read data
	FILE * fp = NULL;
	fp = fopen("in.txt", "r");

	if (fp == NULL){
		printf("ERROR: Unable to open in.txt\n");
		MPI_Finalize();
		return -1;
	}

	int eqSize = 0;
	fscanf(fp, "%d", &eqSize);

	if (SIZE != eqSize){
		printf("ERROR: Size mismatch. Eq: %d != Nodes: %d\n", eqSize, SIZE);
		MPI_Finalize();
		return -1;
	}
	int i, j;
	A = (double *)malloc((SIZE + 1) * sizeof(double));
	buff = (double *)malloc((SIZE + 1) * sizeof(double));
	for(i = 0; i < rank; ++i){
		for(j = 0; j <= eqSize; ++j){
			fscanf(fp, "%f", &A[0]);
		}
		//printf("[%d] : read line %d\n", rank, i+1);
	}
	for(i = 0; i < SIZE + 1; ++i){
		fscanf(fp, "%lf", &A[i]);
	}
	fclose(fp);

	// 0.1 Check data
	
	printf("[%d] : ", rank);
	for(i = 0; i < SIZE + 1; ++i){
		printf("%lf ", A[i]);
	}
	printf("\n");
	

	// = = = = = Faza de eliminare = = = = =
	
	double coef = 0.0;
	if (rank != 0){
		for(i = 0; i < rank; ++i){
			relay(rank-1, rank+1, buff, SIZE+1);
			coef = A[i];
			for (j = 0; j < SIZE+1; ++j)
			{
				A[j] = A[j] - coef * buff[j];
				printf("A[%d][%d] <-- %lf\n", i+1,j,A[j]);
			}
		}
		// de rank ori recv
		//recv from rank-1
		//send to rank+1
		//calculeaza
	}
	double Ar = A[rank];
	for(i=0; i<SIZE+1; ++i){
		A[i] = A[i] / Ar;
	}
	A[rank] = 1.0;
	if (rank < SIZE-1)
		MPI_Send(A, rank+1, MPI_DOUBLE, rank+1, 0xbeef, MPI_COMM_WORLD);
	printf("[%d] --> %lf, %lf, %lf\n", rank, A[0], A[1], A[2]);
	//calculeaza eliminarea termenului propriu
	//send to rank+1
	

	// = = = = = Faza de substitutie = = = =
	if (rank != SIZE-1) {
		for(i = 0; i < SIZE - 1 - rank; ++i) {
			double bk[1];
			//recv de la rank+1
			//send to rank-1 (daca exista)
			relay(rank+1, rank-1, bk, 1);

			//calculeaza
			A[SIZE] = A[SIZE] - A[SIZE - 1 - i] * buff[SIZE];
		}
	}
	//calculeaza termen propriu
	double tp = A[SIZE] / A[rank];
	if (rank > 0)
		MPI_Send(&tp, 1, MPI_DOUBLE, rank-1, 0xbeef, MPI_COMM_WORLD);
	//send to rank-1 (daca exista)


	/*
		for(k = 1; k <= i; ++k){
		//Trimite Aij catre pi,j-1
		MPI_Send(&Aij, 1, MPI_INT, nLeft, 0xbeef, cartComm);
		MPI_Recv(&Aij, 1, MPI_INT, nRight, 0xbeef, cartComm, NULL);
		//Primeste Aij de la pi,j+1
	*/
	printf("x_%d = %lf\n", rank, tp);
	MPI_Finalize();
	return 0;
}


void relay(int src, int dst, double * buff, int bSize){
	int SIZE = 0, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	MPI_Recv(buff, bSize, MPI_DOUBLE, src, 0xbeef, MPI_COMM_WORLD, NULL);
	printf("relay: %d ---> %d\n", src, dst);
	if (dst < SIZE && dst >= 0)
		MPI_Send(buff, bSize, MPI_DOUBLE, dst, 0xbeef, MPI_COMM_WORLD);
}

