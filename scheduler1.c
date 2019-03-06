#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

void scheduler(char* input, char* outfile, int limit, int total){
	int increment = 0, i = 0, k, status, alive = 1, noChildFlag = 1, totalFlag = 0, timeFlag = 0, limitFlag = 0;
	unsigned long nanoSec, life, shmID, seconds = 10000;
	unsigned long * shmPTR;
	char * parameter[32], parameter1[32], parameter2[32], parameter3[32];
	pid_t pid[total], endID = 1; 
	time_t when, when2;
	FILE * fp;
	FILE * outPut;
	unsigned long key;
	srand(time(0));
	key = rand();
	time(&when);
	outPut = fopen(outfile, "a");
	fp = fopen(input, "r");
	//Check for file error.
	if (outPut == NULL){
		perror("Error");
		printf("Output file could not be created.\n");
		exit(EXIT_SUCCESS);
	}
	if (fp == NULL){
		perror("Error");
		printf("File given: %s, error number: %d\n", input, errno);
		printf("No output generated.\n");
		exit(EXIT_SUCCESS);
	}
	fscanf(fp, "%d", &increment);
	shmID = shmget(key, sizeof(unsigned long), IPC_CREAT | IPC_EXCL | 0777);
	shmPTR = (unsigned long *) shmat(shmID, NULL, 0);
	shmPTR[0] = 0;
	for(k = 0; k < total; k++){
		pid[k] = -1;
	}
	signal(SIGINT, intHandler);
	while((alive > 0) && (keepRunning == 1)){
		time(&when2);
		if ((when2 - when) >= 2){
			timeFlag = 1;
		}
		shmPTR[0] += increment;
		if((timeFlag == 0) && (totalFlag == 0) && (limitFlag == 0)){
			if (seconds > 1000){
				if (fscanf(fp, "%li", &seconds) !=EOF){
					fscanf(fp, "%li", &nanoSec);
					fscanf(fp, "%li", &life);
				}
			}
			if(shmPTR[0] >= ((seconds * 1000000000) + nanoSec)){
				if((pid[i] = fork()) == 0){
					sprintf(parameter1, "%li", key);
					sprintf(parameter2, "%li", shmID);
					sprintf(parameter3, "%li", life);
					char * args[] = {parameter1, parameter2, parameter3, NULL};
					fprintf(outPut, "Child process %d launched with %s lifetime.\n", getpid(), parameter3);
					execvp("./child\0", args);
				}
				else{
					if (noChildFlag > 0){
						alive--;
						noChildFlag = 0;
					}
					seconds = 100000;
					nanoSec = 0;
					life = 0;
					alive++;
					i++;
				}
			}
		}
		endID = waitpid(0, &status, WNOHANG | WUNTRACED);
		if (endID == -1){
//			perror("waitpid error");
		}
		else if (endID == 0);
		else{
			for(k = 0; k <= i; k++){
				if(endID == pid[k]){
					alive--;
					if (alive >= 0){
						if (WIFEXITED(status)){
							limitFlag = 0;
							fprintf(outPut, "Child process %d terminated at %li nanoseconds.\n", pid[k], shmPTR[0]);
						}
						else if (WIFSIGNALED(status)){
							printf("Child ended with an uncaught signal, %d.\n", status);
							limitFlag = 0;
						}
						else if (WIFSTOPPED(status)){
							printf("Child process has stopped.\n");
						}
					}
				}
			}
		}
		if (alive >= total){
			totalFlag = 1;
		}
		if (alive >= limit){
			limitFlag = 1;
		}
	}
	if(timeFlag == 1){
		printf("Program has reached its allotted time, exiting.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to time limit.\n",  shmPTR[0]);
	}
	if(totalFlag == 1){
		printf("Program has reached its allotted children, exiting.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to process limit.\n",  shmPTR[0]);
	}
	if(keepRunning == 0){
		printf("Terminated due to ctrl-c signal.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to ctrl-c signal.\n",  shmPTR[0]);
	}
	shmdt(shmPTR);
	fclose(fp);
	fclose(outPut);
}
