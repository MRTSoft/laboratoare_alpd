#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define ID 0
#define FITNESS 1

#define ALEGERE 10
#define CONFIRMARE 20

int main(int argc, char **args) {
	int SIZE = 1; //Get from MPI
	int rank; // rank of the current node
	int data[2];
    int buffer[2];
    
	MPI_Init(&argc, &args);

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //0. Read data
    FILE * fin = NULL;
    fin = fopen("in.txt", "r");
    if (!fin){
        printf("Unable to open in.txt\n");
        MPI_Finalize();
        return 1;
    }
    int i;
    int liderCunoscut = 0;
    int left, right;
    left = (rank - 1 < 0) ? SIZE-1 : rank-1;
    right = (rank + 1) % SIZE;
    data[ID] = rank;

    for(i = 0; i <= rank; ++i){
        fscanf(fin, "%d", &data[FITNESS]);
    }
    fclose(fin);
    printf("[%02d] Fitness: %d\n", rank, data[FITNESS]);

    //1. Send mesaj alegere
    MPI_Send(data, 2, MPI_INT, right, ALEGERE, MPI_COMM_WORLD);
    do{
        MPI_Status status;
        
        MPI_Recv(buffer, 2, MPI_INT, left, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch(status.MPI_TAG){

        case ALEGERE:
            if (buffer[FITNESS] > data[FITNESS]) {
                data[ID] = buffer[ID];
                data[FITNESS] = buffer[FITNESS];
                MPI_Send(data, 2, MPI_INT, right, ALEGERE, MPI_COMM_WORLD);
            } else {
                if (buffer[FITNESS] == data[FITNESS]) {
                    MPI_Send(data, 2, MPI_INT, right, CONFIRMARE, MPI_COMM_WORLD);
                    liderCunoscut = 1;
                }
            }
            break;

        case CONFIRMARE:
            liderCunoscut = 1;
            if (rank+1 != buffer[ID]) {
                MPI_Send(buffer, 2, MPI_INT, right, CONFIRMARE, MPI_COMM_WORLD);
            }
            break;
        
        default:

            printf("[%02d] Unknown tag recived: %d\n", rank, status.MPI_TAG);
        }

    }while(liderCunoscut == 0);

   	//MPI_Send(&Aij, 1, MPI_INT, nLeft, 0xbeef, MPI_COMM_WORLD);
	//MPI_Recv(&Aij, 1, MPI_INT, nRight, 0xbeef, MPI_COMM_WORLD, NULL);
	printf("[%02d]The leader is [%02d] --> %d\n", rank, data[ID], data[FITNESS]);

	MPI_Finalize();
	return 0;
}
