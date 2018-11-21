#include <stdio.h>
#include <omp.h>

#define SIZE 16
#define DEPTH 3

int main() {
	int i, k, j;
	int threads = 8;
	int A[SIZE+1];
	for(i = SIZE/2; i <= SIZE; ++i){
		A[i] = i - SIZE/2 + 1;
	}
	for (k = DEPTH-1; k >= 0; --k){
#pragma omp parallel for num_threads(SIZE/2)
		for(j = 1<<k; j < 1<<(k+1); ++j){
			A[j] = A[2*j] + A[2*j+1];
			//printf("[%d-%d] indexes: %d %d %d\n", k, j, j, 2*j, 2*j+1);
		}
	}

	for(i=1; i<=SIZE; ++i){
		printf("A[%d] = %d\n", i, A[i]);
	}
	return 0;
}
