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
int pid_redirect;
char argv[BUFFER_SIZE];
char *inFile = NULL;
char *outFile = NULL;
char *errFile = NULL;
char *currentArg;
char *previousArg;
int foundFirstArg = 0;
char firstArg[BUFFER_SIZE];

int main () {
	while (1) {
		//get argument input
		printf("# ");
		fgets(argv, BUFFER_SIZE, stdin);
		
		//argument parsing
		strtok(argv, "\n");
		currentArg = strtok(argv, whitespace);
		strcat(firstArg, currentArg);
		while (currentArg != NULL) {
			previousArg = currentArg;
			currentArg = strtok(NULL, whitespace);
			//load file redirection locations
			if (strcmp(previousArg, "<") == 0) {
				inFile = currentArg;
			}
			if (strcmp(previousArg, ">") == 0) {
				outFile = currentArg;
			}
			if (strcmp(previousArg, "2>") == 0) {
				errFile = currentArg;
			}
			if (currentArg != NULL) {	
				//find first arg
				if (foundFirstArg == 0
					&& (strcmp(currentArg, "<") == 0
						|| strcmp(currentArg, ">") == 0
						|| strcmp(currentArg, "2>") == 0)) {
					printf("firstArg: %s\n", firstArg);
					foundFirstArg = 1;
				} else if (foundFirstArg == 0) {
					strcat(firstArg, " ");
					strcat(firstArg, currentArg);
				}
			}
		}

		//replace spaces in argv with null terminating chars
		int i = 0;
 		while(argv[i] != '\0') {
        	if(argv[i] == ' ') {
            	argv[i] = '\0';
        	}
        	i++;
		}
	/*	
		//fork for file redirection
		pid_redirect = fork();
		if (pid_redirect == 0) {
			//child code
						
		}
*/
		// reset args, get ready for next user input
		strcpy(firstArg, "");
		foundFirstArg = 0;
	}
}
