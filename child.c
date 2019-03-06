#include<stdio.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>


int main(int argc, char * argv[]){
	char * ptr;
	unsigned long * shmPTR;
	key_t key = strtoul(argv[0], &ptr, 10);
	unsigned long life = strtoul(argv[2], &ptr, 10);
	unsigned long shmID = strtoul(argv[1], &ptr, 10);
	unsigned long existence;
	shmID = shmget(key, sizeof(unsigned long), O_RDONLY);
	shmPTR = (unsigned long *) shmat(shmID, NULL, 0);
	printf("Child entered at %li with life %li.\n", shmPTR[0], life);
	existence = shmPTR[0] + life;
	printf("Exists until %li.\n", existence);
	while(shmPTR[0] < existence);
	printf("Child exiting at %li.\n", shmPTR[0]);
	shmdt(shmPTR);
	return 0;
}
