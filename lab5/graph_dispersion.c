#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int sendToNeighbors(
		const char * msg, 
		int dest[], 
		int destCount, 
		MPI_Comm comm, 
		int src,
		int nodeRoot) 
{
	int i;
	for(i = 0; i < destCount; ++i) {
		if (dest[i] != nodeRoot) {
			MPI_Send(msg, strlen(msg)+1, MPI_CHAR, dest[i], 16, comm);
			printf("[%d] >>> [%d]\n", src, dest[i]);
		}
	}
}

int waitMessage(
		char * buffer,
		int bufferSize,
		MPI_Comm comm)
{
	int source;
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Status status;
	MPI_Recv(buffer, bufferSize, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
	source = status.MPI_SOURCE;
	printf("[%d] <<< [%d] --- %s\n", rank, source, buffer);
	return source;
}

int main(int argc, char ** args) {
	MPI_Init(&argc, &args);
	int rank;
	int src, dst;
	// Send from root to all nodes
	int graphSize = 9; 	// Number of nodes in the graph
	int index[] = { 	// Node degrees
		 1, // Node 0 - 1 neighbours
		 2, // Node 1 - 1 neighbours
		 5, // Node 2 - 3 neighbours
		 7, // Node 3 - 2 neighbours
		10, // Node 4 - 3 neighbours
		11, // Node 5 - 1 neighbours
		14, // Node 6 - 2 neighbours
		15, // Node 7 - 1 neighbours
		16, // Node 8 - 1 neighbours
	};
	int edges[] = {		// Graph edges
		4,			// n0
		6,			// n1
		4, 7, 8,	// n2
		4, 6,		// n3
		0, 2, 3,	// n4
		6,			// n5
		1, 3, 5,	// n6
		2,			// n7
		2			// n8
	};
	MPI_Comm graphComm;
	MPI_Graph_create(MPI_COMM_WORLD, graphSize, index, edges, 0, &graphComm);
	int nCount; //number of neighbors
	int neighbors[9]; //neighbors of node [rank]
	
	MPI_Comm_rank(graphComm, &rank);
	MPI_Graph_neighbors_count(graphComm, rank, &nCount);
	MPI_Graph_neighbors(graphComm, rank, graphSize, neighbors);
	char * M;//Message to be sent
	const int ROOT = 4;
	const int bufferSize = 30;
	M = (char*) malloc(bufferSize * sizeof(char));
	if (rank == ROOT) {
		strncpy(M, "Sample message from root.", bufferSize);
		sendToNeighbors(M, neighbors, nCount, graphComm, rank, ROOT);
	}
	else {
		if (nCount == 1) { // This is a leaf
			waitMessage(M, bufferSize, graphComm);
		} else { // This is an internal node
			int parent;
			parent = waitMessage(M, bufferSize, graphComm);
			sendToNeighbors(M, neighbors, nCount, graphComm, rank, parent);
		}
	}

	MPI_Finalize();
	return 0;
}
