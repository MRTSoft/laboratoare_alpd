#include <omp.h>
#include <stdio.h>

#define SIZE 8

void compress(int R[2 * SIZE - 1][SIZE], int col);

int main(){
	int A[SIZE] = {20,70,30,10,40,60,80,50};
	int R[2*SIZE-1][SIZE];
	int i,j;
	int n = SIZE;
	//1. faza de comparare
	//R[i+n-1, j] = A[i] < A[j]
	for(i = 0 ; i < SIZE; ++i){
#pragma omp parallel for num_threads(SIZE)
		for(j=0; j < SIZE; ++j) {
			R[i+n-1][j] = A[i] < A[j] ? 1 : 0;
		}
	}

	//Compression phase
#pragma omp parallel for num_threads(SIZE)
	for(j = 0; j < SIZE; ++j){
		compress(R, j);
	}

#pragma omp parallel num_threads(SIZE)
	{
		int rank;
		rank = omp_get_thread_num();
		int aux = A[rank];
#pragma omp barrier
		A[R[0][rank]] = aux;
	}
	

	for (i = 0; i < 2*SIZE-1; ++i){
		for(j=0; j < SIZE; ++j){
			printf("%d ", R[i][j]);
		}
		printf("\n");
	}

	for (i = 0; i < SIZE; ++i){
		printf("%d ", A[i]);
	}
	printf("\n");
	return 0;
}

void compress(int R[2*SIZE-1][SIZE], int col){
	int i;
	for (i = SIZE-2; i >= 0; --i){
		R[i][col] = R[2*i+1][col] + R[2*i+2][col];
	}
}
