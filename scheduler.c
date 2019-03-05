#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

static void child(long [], long);

void scheduler(char* input, char* outfile, int limit, int total){
	int increment = 0, i = 0, k, status, seconds, alive = 1, noChildFlag = 1;
	long nanoSec, life, shmID, timer;
	long * shmPTR;
	pid_t pid[total], endID = 1;
	time_t when;
	FILE * fp;
	time(&when);
	fp = fopen(input, "r");
	//key_t key = ftok("shfile", 64);
	//Check for file error.
	if (fp == NULL){
		perror("Error");
		printf("File given: %s, error number: %d\n", input, errno);
		printf("No output generated.\n");
	}
	fscanf(fp, "%d", &increment);
	shmID = shmget(IPC_PRIVATE, sizeof(long), IPC_CREAT | IPC_EXCL | 0777);
	shmPTR = (long *) shmat(shmID, NULL, 0);
	shmPTR[0] = increment;
	printf("Timer1: %li\n", shmPTR[0]);
	while(alive > 0){
		if (fscanf(fp, "%d", &seconds) == 1){
			fscanf(fp, "%li", &nanoSec);
			fscanf(fp, "%li", &life);
			printf("Seconds: %d\n", seconds);
			printf("Nanoseconds: %li\n", nanoSec);
		}
		while(shmPTR[0] < (seconds * 1000000000) + nanoSec){
			printf("Goes until: %li\n", (shmPTR[0]+life));
			printf("endID: %d; pid: %d\n", endID, pid);
			shmPTR[0] += increment;
			sleep(1);
		}
		if((pid[i] = fork()) == 0){
			child(shmPTR, life);
		}
		else{
			printf("Child %d spawned.\n", pid[i]);
			if (noChildFlag > 0){
				alive--;
				noChildFlag = 0;
			}
			alive++;
			endID = waitpid(pid, &status, WNOHANG | WUNTRACED);
			if (endID == -1){
				perror("waitpid error");
			}
			else if (endID == 0){
				printf("Parent waiting on child %d.\n", pid);
			}
			else{
				for(k = 0; k <= i; k++){
					if(endID == pid[k]){
						alive--;
						if (WIFEXITED(status)){
							printf("Child ended normally.\n");
						}
						else if (WIFSIGNALED(status)){
							printf("Child ended with an uncaught signal.\n");
						}
						else if (WIFSTOPPED(status)){
							printf("Child process has stopped.\n");
						}
					}
				}
			i++;
			}
		}
		printf("Timer: %li\n", shmPTR[0]);
		shmdt(shmPTR);
	}
}

void child(long sharedMem[], long life){
	printf("Child entered at %li.\n", sharedMem[0]);
	long existence = sharedMem[0] + life;
	printf("Exists until %li.\n", existence);
	while(sharedMem[0] < existence);
	printf("Child exiting at %li.\n", sharedMem[0]);
	shmdt(sharedMem);
	exit(EXIT_SUCCESS);
}
