#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "master.h"


enum phase_t {NONE, MAP, REDUCE};
static enum phase_t phase = NONE;

static DIR * input_q = NULL;

int waitForWorker(){
	MPI_Status status;
	char data;
	MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, TAG_FREE, MPI_COMM_WORLD, &status);
	return status.MPI_SOURCE;
}

int runMaster(const char * src, const char * interim, const char * dst) {
	//code for master
	phase = MAP;

	input_q = opendir(src);

	if (input_q == NULL){
		return ERROR_INPUT;
	}
	struct dirent * entry;
	char * path = (char*) malloc(sizeof(char) * 250);
	int freeWorker = -1;

	//Map phase
	//Create a filename with the word if not exist
	//If exist append the source to it

	printf("= = = =   MAP phase   = = = =\n");
	while ((entry = readdir(input_q)) != NULL){
		if (entry->d_type != DT_DIR){
			freeWorker = waitForWorker();
			sprintf(path, "%s/%s", src, entry->d_name);
			MPI_Send(path, strlen(path)+1, MPI_CHAR, freeWorker, TAG_MAP, MPI_COMM_WORLD);
		}
	}
	closedir(input_q);

	//Reduce phase
	//Count how often a source appears in the file
	//(hashmap provided by Pete Worden: https://github.com/petewarden/c_hashmap)
	//Outputs a single file containing the reverse index
	//BARRIER - Wait for all workers to finish
	
	int SIZE = -1;
	int i;
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	

	// = = = = = = B A R I E R = = = = = = 
	for(i = 0; i < SIZE-1; ++i){
		waitForWorker();
	}

	printf("= = = = REDUCE phase  = = = =\n");

	input_q = opendir(interim);
	if (input_q == NULL){
		return ERROR_INPUT;
	}
	int busyWorkers = 0;
	entry = readdir(input_q);
	for(i = 1; i < SIZE || (entry != NULL); ++i, entry = readdir(input_q)){
		if (entry->d_type == DT_DIR){
			continue;
		}
		sprintf(path, "%s/%s", interim, entry->d_name);
		if (i < SIZE) {
			MPI_Send(path, strlen(path)+1, MPI_CHAR, i, TAG_REDUCE, MPI_COMM_WORLD);
			busyWorkers++;
		}
		else {
			i = SIZE+1;
			freeWorker = waitForWorker();
			MPI_Send(path, strlen(path)+1, MPI_CHAR, freeWorker, TAG_REDUCE, MPI_COMM_WORLD);
		}
	}

	closedir(input_q);

	//+ + + + BARIER + + + +
	while (busyWorkers > 0){
		waitForWorker();
		busyWorkers--;
	}

	// = = = = = = Second phase = = = = = = = 
	
	
	//Close the workers
	//We need to close size-1 workers
	for(i = 1; i < SIZE; ++i) {
		//freeWorker = waitForWorker();
		//We don't actually have to wait for them to finish
		//since we're using a syncron method
		char data = 'c';
		MPI_Send(&data, 1, MPI_CHAR, i, TAG_STOP, MPI_COMM_WORLD);
		printf("Closing worker #%03d\n", i);
	}
	return 0;
}
