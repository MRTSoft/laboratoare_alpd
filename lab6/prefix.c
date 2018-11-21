#include <stdio.h>
#include <omp.h>

#define SIZE 16
#define DEPTH 3
#define a0 5
#define ratio 2

int main() {
	int i, k, j;
	int A[SIZE+1];
	int B[SIZE+1];

	A[SIZE/2] = a0;
	for(i = SIZE/2 + 1; i <= SIZE; ++i){
		A[i] = ratio;
	}

	for (k = DEPTH-1; k >= 0; --k){
#pragma omp parallel for
		for(j = 1<<k; j < 1<<(k+1); ++j){
			A[j] = A[2*j] + A[2*j+1];
			//printf("[%d-%d] indexes: %d %d %d\n", k, j, j, 2*j, 2*j+1);
		}
	}
	B[1] = A[1];
	for (k = 1; k <= DEPTH; k++){
#pragma omp parallel for
		for(j = 1<<k; j < 1<<(k+1); ++j){
			if (j % 2 == 1){
				B[j] = B[(j-1)/2];
			}
			else {
				B[j] = B[j/2] - A[j+1];
			}
		}
	}
	for(i=SIZE/2; i<SIZE; ++i){
		printf("B[%d] = %d\n", i, B[i]);
	}
	return 0;
}
