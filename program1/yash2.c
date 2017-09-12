#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const int BUFFER_SIZE = 2001;
const char whitespace[2] = " ";

char argv[BUFFER_SIZE];
char *inFile = NULL;
char *outFile = NULL;
char *errFile = NULL;
char *currentArg;
char *previousArg;

int main () {
	while (1) {
		printf("# ");
		fgets(argv, BUFFER_SIZE, stdin);
		strtok(argv, "\n");
		currentArg = strtok(argv, whitespace);
		while (currentArg != NULL) {
			printf("The currentArg: %s\n", currentArg);
			previousArg = currentArg;
			currentArg = strtok(NULL, whitespace);
			//load file redirection locations
			if (strcmp(previousArg, "<") == 0) {
				inFile = currentArg;
				printf("inFile: %s\n", inFile);
			}
			if (strcmp(previousArg, ">") == 0) {
				outFile = currentArg;
				printf("outFile: %s\n", outFile);
			}
			if (strcmp(previousArg, "2>") == 0) {
				errFile = currentArg;
				printf("errFile: %s\n", errFile);
			}
		}
		/*
		if (inFile != NULL && outFile != NULL && errFile != NULL) {
			pid_redirect = fork();
			if (pid_redirect == 0) {
				
			}
		} */

	}
}
