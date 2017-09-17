#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int pipefd[2];
int status, pid_ch1, pid_ch2, pid;

const int BUFFER_SIZE = 2001;
char argv[BUFFER_SIZE];
const char whitespace[2] = " ";
char *currentArg;
char *previousArg;
int foundFirstArg = 0;
char firstArg[BUFFER_SIZE];

int main() {
	while (1) {
		// Get the input
		printf("# ");
		fgets(argv, BUFFER_SIZE, stdin);
		strtok(argv, "\n");
		// Separate the arguments
		currentArg = strtok(argv, whitespace);
		strcat(firstArg, currentArg);
		while (currentArg != NULL) {
//			printf("The currentArg: %s\n", currentArg);
			currentArg = strtok(NULL, whitespace);
			if (foundFirstArg == 0
				&& (strcmp(currentArg, "<") == 0
					|| strcmp(currentArg, ">") == 0
					|| strcmp(currentArg, "2>") == 0)) {
				printf("Firstarg: %s\n", firstArg);
				foundFirstArg = 1;	
		/*		pid_ch1 = fork();
				if (pid_ch1 == 0) {
					while (currentArg != NULL) {
						previousArg = currentArg;
						if (strcmp(currentArg, "<") == 0) {
							currentArg = strtok(NULL, whitespace);
							int in = open(currentArg, O_RDONLY);
							dup2(in, STDIN_FILENO);
						} else if (strcmp(currentArg, ">") == 0) {
							currentArg = strtok(NULL, whitespace);
							int out = open(currentArg, O_WRONLY);
							dup2(out, STDOUT_FILENO);
						} else if (strcmp(currentArg, "2>") == 0) {
							currentArg = strtok(NULL, whitespace);
							int err = open(currentArg, O_WRONLY);
							dup2(err, STDERR_FILENO);
						} else {
							currentArg = strtok(NULL, whitespace);
						}
					}
	//				execvp(firstArg, argv);*/
				}
			} else if (foundFirstArg == 0) {
				strcat(firstArg, " ");
				strcat(firstArg, currentArg);
			}	
		}
		strcpy(firstArg, "");
		foundFirstArg = 0;
	}
	
}
