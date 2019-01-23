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

	int SIZE = -1;
	int i;
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	

	// = = = = = = B A R I E R = = = = = = 
	for(i = 1; i < SIZE; ++i){
		freeWorker = waitForWorker();
		MPI_Send(&i, 1, MPI_INT, freeWorker, TAG_SYNC, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);// Wait for all workers to finish the map phase

	printf("= = = = REDUCE phase  = = = =\n");
	// Get a word from the interim folder
	// Assign word to free node

	char cmd[MAX_PATH_LEN] = "";
	snprintf(cmd, MAX_PATH_LEN, "ls -A %s/*.txt", interim);
	FILE * fileList = openReadPipe(cmd);
	if (fileList == NULL){
		fprintf(stderr, "[MASTER] Failed to execute piped command.\n");
		return ERROR_INPUT;
	}

	char * word = (char*) malloc(sizeof(char) * MAX_PATH_LEN);
	char * oldWord = (char*) malloc(sizeof(char) * MAX_PATH_LEN);
	char * delimitator = NULL;
	char * filePath = (char*) malloc(sizeof(char) * MAX_PATH_LEN);
	int wordLen = 0;
	word[0] = oldWord[0] = '\0';

	while (fgets(filePath, MAX_PATH_LEN, fileList) != NULL){
		//freeWorker = waitForWorker();
		strncpy(oldWord, word, strlen(word)+1);
		delimitator = memchr(filePath, DELIM_CHAR, strlen(filePath));
		wordLen = delimitator - filePath + 1;
		strncpy(word, filePath, wordLen);
		word[wordLen] = '\0';
		if (strncmp(word, oldWord, MAX_PATH_LEN) != 0){
			//printf("Processing the word: %s\n", word);
			//MPI_Send(path, strlen(path)+1, MPI_CHAR, freeWorker, TAG_MAP, MPI_COMM_WORLD);
		}
	}
	pclose(fileList);

	//Close the workers
	//We need to close size-1 workers
	for(i = 1; i < SIZE; ++i) {
		char data = 'c';
		MPI_Send(&data, 1, MPI_CHAR, i, TAG_STOP, MPI_COMM_WORLD);
		printf("Closing worker #%03d\n", i);
	}
	return 0;
}
