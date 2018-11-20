#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROOT 6
#define SIZE 3 //Size of the hipercupe (dimensions)

int main(int argc, char ** args) {
	int rank;
	int mask;
	char * M;
	int virtual_id;
	int k = SIZE;
	int dest;
	int src;

	mask = (1<<k) - 1;
	M = (char*) malloc(50*sizeof(char));
	MPI_Init(&argc, &args);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	virtual_id = rank ^ ROOT;
	if (virtual_id == ROOT) {
		strncpy(M, "Sample message", 50);
	}
	for(k = SIZE - 1; k>=0; --k){
		mask = mask ^ (1<<k);
		if ((virtual_id & mask) == 0) {
			// Nodul va participa la comunicare tura asta
			// Determinam daca va primi sau va trimite mesajul
			if ((virtual_id & (1<<k)) == 0) {
				// Nodul este sursa
				dest = ( virtual_id ^ (1<<k) ) ^ ROOT;
				MPI_Send(M, strlen(M)+1, MPI_CHAR, dest, 16, MPI_COMM_WORLD);
				printf("[%d] -> [%d] --- %s\n", virtual_id ^ ROOT, dest, M);
			}
			else {
				src = ( virtual_id ^ (1<<k) ) ^ ROOT;
				MPI_Recv(M, 50, MPI_CHAR, src, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
				printf("[%d] <- [%d] --- %s\n", virtual_id ^ ROOT, src, M);
			}
		}
	}
	MPI_Finalize();
	return 0;
}
