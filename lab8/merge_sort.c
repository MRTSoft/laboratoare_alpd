#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum comp_t {PLUS, MINUS} comp_t;

int compareValue(int rank, int partener, int value, int other_value, comp_t compType){
	/*
	if (
			(compType == PLUS && (rank < partener)) || 
			(compType == MINUS && (rank > partener))
		){
		return (value < other_value) ? value : other_value;
	}
	return (value < other_value) ? other_value : value;
	*/
	int min = (value < other_value) ? value : other_value;
	int max = (value > other_value) ? value : other_value;

	printf("SRC_%d COMP: [%d] --> [%d] with %d(%d and %d)\n", rank, rank, partener, compType, value, other_value);
	if (compType == PLUS){
		if (rank < partener){
			return min;
		}
		else {
			return max;
		}
	}
	if (compType == MINUS){
		if (rank < partener){
			return max;
		}
		else {
			return min;
		}
	}
	return -1;
}

int sendAndRecv(int rank, int partener, int value, int tag){
	int other_value = 0;
	if (rank < partener){
		//send value to partener
		MPI_Send(&value, 1, MPI_INT, partener, tag, MPI_COMM_WORLD);
		printf("SRC_%d %d: [%d] >>> [%d] -- %d\n", rank, tag, rank, partener, value);
		//recv other_value from partener
		MPI_Recv(&other_value, 1, MPI_INT, partener, tag, MPI_COMM_WORLD, NULL);
		printf("SRC_%d %d: [%d] <<< [%d] -- %d\n", rank, tag, rank, partener, other_value);
	}
	else {
		//recv other_value from partener
		MPI_Recv(&other_value, 1, MPI_INT, partener, tag, MPI_COMM_WORLD, NULL);
		printf("SRC_%d %d: [%d] <<< [%d] -- %d\n", rank, tag, rank, partener, other_value);
		//send value to partener
		MPI_Send(&value, 1, MPI_INT, partener, tag, MPI_COMM_WORLD);
		printf("SRC_%d %d: [%d] >>> [%d] -- %d\n", rank, tag, rank, partener, value);
	}
	return other_value;

}


int main(int argc, char **args) {
	int V[32] = {
		15, 11, 12, 18, 17, 14, 13, 16, 
		24, 26, 25, 20, 22, 21, 23, 19, 
		37, 38, 33, 39, 41, 27, 28, 29, 
		36, 40, 35, 31, 32, 34, 30, 42
	};//Pseudo random data to be sorted

	int SIZE = 1; //Get from MPI
	int i, j, k;
	int rank; // rank of the current node

	MPI_Init(&argc, &args);
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if ((SIZE & (SIZE - 1)) != 0) {
		if (rank == 0) {
			printf("You need to have a power of two nodes running!\n");
		}
		MPI_Finalize();
		return 0;
	}

	//0. Distribute the elements to the nodes
	int value;
	int other_value;
	int partener;
	comp_t compType;
	value = V[rank];
	int exp = 0; // 2^exp = SIZE
	while ( (1 << exp) != SIZE ){
		exp ++;
	}

	//1. Create the bitonic sequence
	// To do this we need exp-1 rounds
	int mask = SIZE-1;
	for (i = 0; i<(exp-1); ++i) {
		compType = ((rank >> (i+1)) & 1) == 0 ? PLUS : MINUS;
		// The i-th bit of our rank determines the comparator

		for (j = i; j >= 0; --j){ //in each round we have exactly i+1 exchanges
			//every node participates in the exchange;
			//As a rule, the node with the lowest rank sends then recived. The partener recives then sends.
			//1.1 Who is the partener?
			// To find the partener we flip the j bit
			partener = rank ^ (1<<j);
			other_value = sendAndRecv(rank, partener, value, 1000 + i*100+j);
			value = compareValue(rank, partener, value, other_value, compType);
		}
	}

	if (rank == 0){
		printf("\n- - - - SORT phase - - - - \n");
	}
	//2. Sort bitonic sequence
	compType = PLUS; // Comparator type remains the same for all nodes
	for(i = exp; i>0; --i){
		partener = rank ^ (1 << (i-1)); // flip the i-1 bit to find the partener
		other_value = sendAndRecv(rank, partener, value, i);
		value = compareValue(rank, partener, value, other_value, compType);
		if (rank == 0){
			printf("\n");
		}
	}

	printf("SRC_%d FINAL VALUE [%d] -- %d\n", rank, rank, value);

	MPI_Finalize();
	return 0;
}
