#include<stdio.h>
#include<fcntl.h>
#include<getopt.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include "help.h"
#include "scheduler.h"


int main(int argc, char * argv[]){
	key_t key = (unsigned long)argv[1];
	unsigned long life = (unsigned long)argv[3];
	unsigned long shmID = (unsigned long)argv[2];
	unsigned long existence;
	unsigned long * shmPTR = (unsigned long *) shmat(shmID, NULL, 0);
	printf("Child entered at %li with life %li.\n", shmPTR[0], life);
	existence = shmPTR[0] + life;
	printf("Exists until %li.\n", existence);
	while(shmPTR[0] < existence);
	printf("Child exiting at %li.\n", shmPTR[0]);
	shmdt(shmPTR);
	return 0;
}
