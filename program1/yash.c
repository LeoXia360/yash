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
int pipefd[2];
int status, pid_ch1, pid_ch2, pid;
int pid_redirect;
int pid_default;
char argv[BUFFER_SIZE];
char *inFile = NULL;
char *outFile = NULL;
char *errFile = NULL;
char *currentArg;
char *previousArg;
int foundFirstArg = 0;
char firstArg[BUFFER_SIZE];
char* args[BUFFER_SIZE];
char* args2[BUFFER_SIZE];
int isPipe = 0;
int isFileRedirection = 0;
int mostRecentPID = -1;
int processStack[100];
const int MAX_PROCESS_STACK_SIZE = 100;
int processStackTop = -1;

static void sig_do_nothing(int signo) {
}
int isEmpty();

int isFull();

int peek();

int pop();

void push(int value); 
 
void run_file_redirection();

void run_pipe();

void run_default_exec();

static void sig_int(int signo) {
  printf("Sending signals to group:%d\n",pid_ch1); // group id is pid of first in pipeline
  kill(-pid_ch1,SIGINT);
}
static void sig_tstp(int signo) {
  printf("Sending SIGTSTP to group:%d\n",pid_ch1); // group id is pid of first in pipeline
  kill(-pid_ch1,SIGTSTP);
  push(pid_ch1);
  return;
}

void init_main() {
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

int main () {

    if (signal(SIGINT, sig_do_nothing) == SIG_ERR)
		printf("signal(SIGINT) error");
    if (signal(SIGTSTP, sig_do_nothing) == SIG_ERR)
		printf("signal(SIGTSTP) error");
	while (1) {
		//get argument input
		printf("# ");
		if(fgets(argv, BUFFER_SIZE, stdin) == NULL) {
			exit(1);
		}
		
		//argument parsing
		int count = 0;
		int count2 = 0;
		strtok(argv, "\n");
		currentArg = strtok(argv, whitespace);
		args[count] = currentArg;
	//	printf("args[%d]: %s\n", count, args[count]);
		count += 1;
		while (currentArg != NULL) {
			previousArg = currentArg;
			currentArg = strtok(NULL, whitespace);
			
			if (currentArg == NULL && strcmp(previousArg, "&") == 0) {
				//send to background
			}
			
			if (strcmp(previousArg, "fg") == 0 && mostRecentPID != -1) {
				kill(mostRecentPID, SIGCONT);
				//Set new most recent PID							
				if (pop() != -1) {
					printf("mostRecentPID: %d\n", mostRecentPID);
					if (!isEmpty()) {
						mostRecentPID = peek(); 
					} else {
						mostRecentPID = -1;
					}
				} else {
					printf("Invalid pop, mostRecentPID: %d\n", mostRecentPID);
				}
			}

			//Checking for pipes
			if (strcmp(previousArg, "|") == 0) {
				isPipe = 1;
			}
			if (isPipe == 1) {
				args2[count2] = currentArg;
	//			printf("args2[%d]: %s\n", count2, args2[count2]);
				count2++;
			}

			//load file redirection locations
			if (strcmp(previousArg, "<") == 0) {
				inFile = currentArg;
				isFileRedirection = 1;
			}
			if (strcmp(previousArg, ">") == 0) {
				outFile = currentArg;
				isFileRedirection = 1;
			}
			if (strcmp(previousArg, "2>") == 0) {
				errFile = currentArg;
				isFileRedirection = 1;
			}
			if (currentArg != NULL) {	
				//find first arg
				if (foundFirstArg == 0
					&& (strcmp(currentArg, "<") == 0
						|| strcmp(currentArg, ">") == 0
						|| strcmp(currentArg, "2>") == 0
						|| strcmp(currentArg, "|") == 0)) {
					foundFirstArg = 1;
					args[count] = NULL;
	//				printf("args[%d]: %s\n", count, args[count]);
				} else if (foundFirstArg == 0) {
					args[count] = currentArg;
	//				printf("args[%d]: %s\n", count, args[count]);
					count += 1;
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

		//Create a child for file redirection
		if (isFileRedirection == 1) {
			run_file_redirection();		
		} else if (isPipe == 1) {
			//Create children for pipeing
			run_pipe();
		} else {
			run_default_exec();	
		}

		// reset args, get ready for next user input
		strcpy(firstArg, "");
		foundFirstArg = 0;
		inFile = NULL;
		outFile = NULL;
		errFile = NULL;
		isPipe = 0;
		isFileRedirection = 0;
	}
}

void run_file_redirection() { 
	//Check to see if inFile is valid, if not throw an error
	int isValidInFile = 1;
	if (inFile != NULL) {
		int in = open(inFile, O_RDONLY);
		if (in == -1) {
			isValidInFile = 0;
			printf("Invalid file: %s\n", inFile);
		}
		close(in);
	}
	if (isValidInFile == 1) {
		//fork for file redirection
		pid_redirect = fork();
		if (pid_redirect == 0) {
			//child code
			if (inFile != NULL) {
				int in = open(inFile, O_RDONLY);
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
    		perror("execvp");
			exit(EXIT_FAILURE);
		} else if (pid_redirect == -1) {
			perror("waitpid");
	  		exit(EXIT_FAILURE);
		} else {
			// parent
			wait(NULL);
		}
	}
}

void run_pipe() {
	if (pipe(pipefd) == -1) {
    	perror("pipe");
    	exit(-1);
  	}

  	pid_ch1 = fork();
  	if (pid_ch1 > 0) {
    	printf("Child1 pid = %d\n", pid_ch1);
    	// Parent
    	pid_ch2 = fork();
    	if (pid_ch2 > 0) {
      		printf("Child2 pid = %d\n", pid_ch2);
    		if (signal(SIGINT, sig_int) == SIG_ERR)
				printf("signal(SIGINT) error");
    		if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
				printf("signal(SIGTSTP) error");
      		close(pipefd[0]); //close the pipe in the parent
     		close(pipefd[1]);
      		int count = 0;
      		while (count < 2) {
				// Parent's wait processing is based on the sig_ex4.c
				pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
				// wait does not take options:
				//    waitpid(-1,&status,0) is same as wait(&status)
				// with no options waitpid wait only for terminated child processes
				// with options we can specify what other changes in the child's status
				// we can respond to. Here we are saying we want to also know if the child
				// has been stopped (WUNTRACED) or continued (WCONTINUED)
				if (pid == -1) {
	  				perror("waitpid");
	  				exit(EXIT_FAILURE);
				}
	
				if (WIFEXITED(status)) {
	  				printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));count++;
				} else if (WIFSIGNALED(status)) {
	  				printf("child %d killed by signal %d\n", pid, WTERMSIG(status));count++;
				} else if (WIFSTOPPED(status)) {
	  				printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
					mostRecentPID = pid;
					return;
				} else if (WIFCONTINUED(status)) {
	  				printf("Continuing %d\n",pid);
				}
      		}	
//      		exit(1);
    	}else {
      		//Child 2
      		sleep(1);
      		setpgid(0,pid_ch1); //child2 joins the group whose group id is same as child1's pid
      		close(pipefd[1]); // close the write end
      		dup2(pipefd[0],STDIN_FILENO);
      		execvp(args2[0], args2);  // runs word count
    	}
  	} else {
 	    // Child 1
    	setsid(); // child 1 creates a new session and a new group and becomes leader -
        //   group id is same as his pid: pid_ch1
    	close(pipefd[0]); // close the read end
    	dup2(pipefd[1],STDOUT_FILENO);
    	execvp(args[0], args);  // runs top
  	}
}

void run_default_exec() {
	pid_default = fork();
	if (pid_default == 0) {
		execvp(args[0], args);		
	} else {
		wait(NULL);
	}
}

int isEmpty() {
	if (processStackTop == -1) {
		return 1;
	} else {
		return 0;
	}
}

int isFull() {
	if (processStackTop == MAX_PROCESS_STACK_SIZE) {
		return 1;
	} else {
		return 0;
	}
}

int pop() {
	int value;
	if (!isEmpty()) {
		value = processStack[processStackTop];
		processStackTop -= 1;
		return value;
	}
	return -1;
}

void push(int value) {
	if (!isFull()) {
		processStackTop += 1;
		processStack[processStackTop] = value;
	}
}	

int peek() {
	return processStack[processStackTop];
}













