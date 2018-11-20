#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int sendToParent(
		int value,
		int parent, 
		MPI_Comm comm) 
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	int i;
	MPI_Send(&value, 1, MPI_INT, parent, 16, comm);
	printf("[%d] >>> [%d] -- %d\n", rank, parent, value);
}

// Wait for msgCount messages. 
// The value of buffer is a sum of all recived messages
// Return the sum of neighbors ID that didn't send any messages.
// To use this function for the root node provide an appropriate value for msgCount.
int waitMessages(
		int * buffer,
		int msgCount,
		int neighbors[],
		MPI_Comm comm)
{
	int source, rank, nCount;
	int aux, i, parent = 0;

	MPI_Comm_rank(comm, &rank);
	MPI_Graph_neighbors_count(comm, rank, &nCount);

	for (i = 0; i < nCount; ++i) {
		parent += neighbors[i]; 
	}
	*buffer = 0;

	for (i = 0; i < msgCount; ++i) {
		MPI_Status status;
		MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
		source = status.MPI_SOURCE;
		printf("[%d] <<< [%d] --- %d\n", rank, source, aux);
		*buffer += aux;
		parent -= source;
	}
	return parent;
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
		14, // Node 6 - 3 neighbours
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
	const int ROOT = 4;
	if (rank == ROOT) {
		int val;
		waitMessages(&val, nCount, neighbors, graphComm);
		val += rank;
		printf("- - - - RESULT : %d - - - -\n", val);
	}
	else {
		if (nCount == 1) 
		{ // This is a leaf
			printf("LEAF: ");
			sendToParent(rank, neighbors[0], graphComm);					
		} 
		else 
		{ // This is an internal node
			int val;
			int parent;
			parent = waitMessages(&val, nCount-1, neighbors, graphComm);

			val += rank;
			sendToParent(val, parent, graphComm);
		}
	}
	MPI_Finalize();
	return 0;
}
