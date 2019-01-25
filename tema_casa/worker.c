#include "worker.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


char tmp_dir[MAX_PATH_LEN] = "./data-interim";
char out_dir[MAX_PATH_LEN] = "./data-output";


void sendFree(int master){
	char data = 'f';
	MPI_Send(&data, 1, MPI_CHAR, master, TAG_FREE, MPI_COMM_WORLD);
}

int runWorker(int master) {
	MPI_Status status;
	int keepAlive = 1;
	char buffer[500];

	sendFree(master);

	while (keepAlive == 1){
		MPI_Recv(&buffer, 500, MPI_INT, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch (status.MPI_TAG){
			case TAG_MAP:
				map(buffer);
				sendFree(master);
				break;
			case TAG_REDUCE:
				reduce(buffer);
				sendFree(master);
				break;
			case TAG_STOP:
				keepAlive = 0;
				break;
			case TAG_SYNC:
				MPI_Barrier(MPI_COMM_WORLD);
				sendFree(master);
				break;
			default:
				keepAlive = 0;
		}
	}
	return 0;
}

void appendEntry(const char * entry, const char * source) {
	char path[MAX_PATH_LEN];
	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	sprintf(path, "%s/%s_%s", tmp_dir, entry, strrchr(source, '/')+1);
	//printf("Path of interim file is: %s\n", path);
	int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		//The file already exist
		int oldCount = 0;
		FILE * in = fopen(path, "r");
		fscanf(in, "%d", &oldCount);
		fclose(in);

		FILE * out = fopen(path, "w"); //open in append mode
		if (out != NULL){
			fprintf(out, "%d\n", oldCount + 1);
		}
		fclose(out);
	}
	else {
		close(fd);
		//Use standard C syntax to write stuff
		FILE * out = fopen(path, "w");
		if (out != NULL) {
			fprintf(out, "1\n");
			fclose(out);
		}
	}
}

void map(const char * file){
	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//printf("[%02d] Opening %s ... \n", rank, file);
	FILE * in = fopen(file, "r");
	if (in == NULL){
		printf("[%02d] ERROR -- Unable to open %s\n", file);
		return;
	}
	char word[100];
	char sWord[100];
	while (!feof(in)){
		fscanf(in, "%s", word);
		int isWord = 1;
		char * chr = NULL;
		for(chr = word; (*chr!='\0') && (isWord != 0); chr++){
			isWord = isalnum(*chr);
			//printf("[%2d] chr: %c | isalnum: %d\n", rank, *chr, isalnum(*chr));
			*chr = tolower(*chr);
		}
		if ((strlen(word) > 3) && (isWord != 0)){
			//printf("[%2d] Read |%s| -- %d\n", rank, word, isWord);
			appendEntry(word, file);
		}
	}
	fclose(in);
}

void reduce(const char * word){
	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//printf("[%2d] REDUCING Word: %s\n", rank, word);
	char cmd[MAX_PATH_LEN] = "";
	snprintf(cmd, MAX_PATH_LEN, "ls -A %s/%s*.txt", tmp_dir, word);
	FILE * fileList = openReadPipe(cmd);
	if (fileList == NULL){
		fprintf(stderr, "[%2d] Failed to execute piped command.\n", rank);
		return;
	}
	char cleanWord[MAX_PATH_LEN];
	char filePath[MAX_PATH_LEN];
	char sourceFile[MAX_PATH_LEN];
	char * delimitator;

	strncpy(cleanWord, word, strlen(word));
	cleanWord[strlen(word)-1] = '\0';//remove trailing _
	//printf("[%2d] Clean word is %s\n", rank, cleanWord);
	char rIndexPath[MAX_PATH_LEN];
	sprintf(rIndexPath, "%s/%s.rev", out_dir, cleanWord);
	FILE * reverseIndex = fopen(rIndexPath, "w");
	if (reverseIndex == NULL){
		fprintf(stderr, "[%2d] Unable to create output file: %s\n", rank, rIndexPath);
		pclose(fileList);
		return;
	}

	while (fgets(filePath, MAX_PATH_LEN, fileList) != NULL){
		filePath[strlen(filePath)-1] = '\0';//remove newline at the end
		delimitator = memchr(filePath, DELIM_CHAR, strlen(filePath)) + 1;
		strncpy(sourceFile, delimitator, strlen(delimitator));
		sourceFile[strlen(delimitator)] = '\0';
		//printf("[%2d] path: %s| delim: %s|\n", rank, filePath, sourceFile);
		FILE * in = fopen(filePath, "r");
		if (in == NULL){
			fprintf(stderr, "[%2d] FATAL: Unable to open interim file: %s\n", rank, filePath);
			continue;
		}
		int count = -1;
		fscanf(in, "%d", &count);
		fclose(in);
		fprintf(reverseIndex, "%s %d\n", sourceFile, count);
	}
	fclose(reverseIndex);
	pclose(fileList);
}



