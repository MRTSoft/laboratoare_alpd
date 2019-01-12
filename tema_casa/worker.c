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


char tmp_dir[500] = "./data-interim";


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
			default:
				keepAlive = 0;
		}
	}
	return 0;
}

void filterEntry(const char * entry, char * output){
	int i = 0;
	//turn entry to lowercase 
	for(i = 0; entry[i]; ++i){
		output[i] = tolower(entry[i]);
	}
	output[i] = '\0';
	for (i = 0; output[i]; ++i){
		if (output[i] < 'a' || output[i] > 'z'){
			output[i] = '_';
		}
	}
	int start = -1;
	int end = -1;
	for(i = 0; output[i]; ++i){
		if (output[i] != '_'){
			if (start == -1){
				start = i;
			}
			end = i;
		}
	}
	if (start != -1){
		for(i = 0; start <= end; ++start, ++i){
			output[i] = output[start];
		}
		output[i] = '\0';
	}
	else {
		output[0] = '\0';
	}

}

void appendEntry(const char * entry, const char * source) {
	char path[500];
	char sanitizedEntry[200];
	filterEntry(entry, sanitizedEntry);
	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	sprintf(path, "%s/%s_%s", tmp_dir, sanitizedEntry, strrchr(source, '/')+1);
	//printf("Path of interim file is: %s\n", path);
	int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		//The file already exist
		FILE * out = fopen(path, "a"); //open in append mode
		if (out != NULL){
			fprintf(out, "%s\n", source);
		}
		fclose(out);
	}
	else {
		close(fd);
		//Use standard C syntax to write stuff
		FILE * out = fopen(path, "w");
		if (out != NULL) {
			fprintf(out, "%s\n%s\n", sanitizedEntry, source);
			fclose(out);
		}
	}
}

void map(const char * file){
	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("[%02d] Opening %s ... \n", rank, file);
	FILE * in = fopen(file, "r");
	if (in == NULL){
		printf("[%02d] ERROR -- Unable to open %s\n", file);
		return;
	}
	char word[100];
	char sWord[100];
	while (!feof(in)){
		fscanf(in, "%s", word);
		filterEntry(word, sWord);
		if (strlen(sWord) > 3){
			//printf("Appending %s...\n", word);
			appendEntry(word, file);
		}
	}

	fclose(in);
	
}

typedef struct _record_t {
	char * source;
	int count;
} record_t;

#define MAX_RECORDS 100
#define MAX_RECORD_SIZE 300

void addIndexEntry(const char * fileSrc, const char * source, int count) {
	//TODO read this from the file as wc-1
	
	record_t * records = (record*) malloc(MAX_RECORDS * sizeof(record_t));
	
	// TODO aquire lock on file
	/*
	struct flock fl;
	memset(&fl, 0, sizeof(f1));
	// lock entire file for reading
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	*/
	char buffer[MAX_RECORD_SIZE];
	char auxCount = -1;
	int N = 0;
	FILE * in = fopen(fileSrc, "r");
	while (!feof(in)){
		scanf("%s %d", buffer, auxCount);
		records[N].source = (char *)malloc((strlen(buffer)+1) * sizeof(char));
		records[N].count = auxCount;
		N++;
	}
	fclose(in);

	records[N].source = (char*)malloc((strlen(source)+1) * sizeof(char));
	records[N].count = count;
	N++;

	//sort the list
	
	//rewrite the file
	in = fopen(fileSrc, "w");
	//write the term
	//Write the records
	fclose(in);
}

void reduce(const char * file){
	char term[250];
	char source[250];
	int rank;
	int count = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("[%02d] Reducing %s\n", rank, file);
	FILE * in = fopen(file, "r");
	fgets(term, 250, in);
	while (!feof(in)){
		fgets(source, 250, in); //dummy read to count lines
		count++;
	}
	term[strlen(term)-1] = '\0';//Remove the trailing \n
	strncpy(source, strrchr(file, '/')+1+strlen(term)+1, 250);

	fclose(in);
	char reducedFile[250];
	sprintf(reducedFile, "%s/reduce/%s", tmp_dir, term);
	int fd = open(reducedFile, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		//The file already exist
		FILE * out = fopen(reducedFile, "a"); //open in append mode
		if (out != NULL){
			fprintf(out, "%s %d\n", source, count);
		}
		//append entr
		addIndexEntry(reducedFile, source, count);
		fclose(out);
	}
	else {
		close(fd);
		//Use standard C syntax to write stuff
		FILE * out = fopen(reducedFile, "w");
		if (out != NULL) {
			fprintf(out, "%s\n%s %d\n", term, source, count);
			fclose(out);
		}
	}
}



