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
char* args[BUFFER_SIZE];

int main () {
	while (1) {
		//get argument input
		printf("# ");
		fgets(argv, BUFFER_SIZE, stdin);
		
		//argument parsing
		int count = 0;
		strtok(argv, "\n");
		currentArg = strtok(argv, whitespace);
		args[count] = currentArg;
		//printf("%s\n", args[count]);
		count += 1;
		strcat(firstArg, currentArg);
		while (currentArg != NULL) {
			previousArg = currentArg;
			currentArg = strtok(NULL, whitespace);
			//args[count] = currentArg;
			//printf("%s\n", args[count]);
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
					//printf("firstArg: %s\n", firstArg);
					foundFirstArg = 1;
				} else if (foundFirstArg == 0) {
					//strcat(firstArg, " ");
					//strcat(firstArg, currentArg);
					args[count] = currentArg;
//					printf("%s\n", args[count]);
					count += 1;
				}
			}
		}
		args[count] = NULL;
//		printf("%s\n", args[count]);

		//replace spaces in argv with null terminating chars
		int i = 0;
 		while(argv[i] != '\0') {
        	if(argv[i] == ' ') {
            	argv[i] = '\0';
        	}
        	i++;
		}
//		printf("argv: %s\n", argv);
		
//		char currentDir[3] = "./";
//		strcat(currentDir, inFile);
//		printf("inFile: %s\n", inFile);
//		printf("outFile: %s\n", outFile);
//		printf("errFile: %s\n", errFile);
	
		//fork for file redirection
		pid_redirect = fork();
		if (pid_redirect == 0) {
			//child code
			if (inFile != NULL) {
				int in = open(inFile, O_RDONLY | O_CREAT, 0666);
				dup2(in, STDIN_FILENO);
				close(in);
			}
			if (outFile != NULL) {
				int out = open(outFile, O_WRONLY | O_CREAT, 0666);
				dup2(out, STDOUT_FILENO);
				close(out);
			}
			if (errFile != NULL) {
				int err = open(errFile, O_WRONLY | O_CREAT, 0666);
				dup2(err, STDERR_FILENO);
				close(err);
			}				
			execvp(args[0], args);					
    		exit(0);
		} else if (pid_redirect == -1) {
			perror("waitpid");
	  		exit(EXIT_FAILURE);
		}
		
		// reset args, get ready for next user input
		strcpy(firstArg, "");
		foundFirstArg = 0;
		inFile = NULL;
		outFile = NULL;
		errFile = NULL;
	}
}
