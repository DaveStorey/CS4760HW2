#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>


void scheduler(char* input, char* outfile, int limit, int total){
	int increment = 0, i = 0, k, status, alive = 1, noChildFlag = 1;
	unsigned long nanoSec, life, shmID, seconds = 10000;
	unsigned long * shmPTR;
	char * parameter[32], parameter1[32], parameter2[32], parameter3[32];
	pid_t pid[total], endID = 1; 
	time_t when, when2;
	FILE * fp;
	key_t key;
	srand(time(0));
	key = rand();
	time(&when);
	fp = fopen(input, "r");
	//Check for file error.
	if (fp == NULL){
		perror("Error");
		printf("File given: %s, error number: %d\n", input, errno);
		printf("No output generated.\n");
	}
	fscanf(fp, "%d", &increment);
	shmID = shmget(key, sizeof(unsigned long), IPC_CREAT | IPC_EXCL | 0777);
	shmPTR = (unsigned long *) shmat(shmID, NULL, 0);
	shmPTR[0] = 0;
	for(k = 0; k < total; k++){
		pid[k] = -1;
	}
//	while(alive > 0){
	for(int j = 0; j < 3; j++){
		time(&when2);
		if ((when2 - when) >= 2){
			printf("Program has exceeded its allotted time, exiting.\n");
			shmdt(shmPTR);
			wait(NULL);
			exit(EXIT_SUCCESS);
		}
		if (seconds > 1000){
			if (fscanf(fp, "%d", &seconds) !=EOF){
				fscanf(fp, "%li", &nanoSec);
				fscanf(fp, "%li", &life);
			}
		}
		shmPTR[0] += increment;
		if(shmPTR[0] >= ((seconds * 1000000000) + nanoSec)){
			if((pid[i] = fork()) == 0){
				sprintf(parameter1, "%li", key);
				sprintf(parameter2, "%li", shmID);
				sprintf(parameter3, "%li", life);
				char * args[] = {parameter1, parameter2, parameter3, NULL};
				execvp("./child\0", args);
			}
			else{
				printf("Child %d spawned.\n", pid[i]);
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
		endID = waitpid(0, &status, WNOHANG | WUNTRACED);
		if (endID == -1){
//			perror("waitpid error");
		}
		else if (endID == 0);
		else{
			for(k = 0; k <= i; k++){
				if(endID == pid[k]){
					alive--;
					if (alive > 0){
						if (WIFEXITED(status)){
							printf("Child ended normally.\n");
						}
						else if (WIFSIGNALED(status)){
							printf("Child ended with an uncaught signal, %d.\n", status);
						}
						else if (WIFSTOPPED(status)){
							printf("Child process has stopped.\n");
						}
					}
				}
			}
		}
		if (alive > total){
			printf("Program has exceeded its allotted children, exiting.\n");
			shmdt(shmPTR);
			wait(NULL);
			exit(EXIT_SUCCESS);
		}
	}
	wait(NULL);
	shmdt(shmPTR);
}
