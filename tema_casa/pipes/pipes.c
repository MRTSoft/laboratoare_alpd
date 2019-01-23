#include <stdio.h>

#define PATH_MAX 500

FILE *fp;
int status;
char path[PATH_MAX];

int main() {

	fp = popen("ls *", "r");
	if (fp == NULL) {
		/* Handle error */;
		return 2; 
	}

	while (fgets(path, PATH_MAX, fp) != NULL)
		printf("%s", path);

	status = pclose(fp);
	if (status == -1) {
		/* Error reported by pclose() */
		return 2;
	} else {
		/* Use macros described under wait() to inspect `status' in order
		   to determine success/failure of command executed by popen() */
	}
	return 0;
}
