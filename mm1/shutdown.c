// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Memory Manager 1
// Due Date: Apr 12, 2024
// Sources: Chat-GPT for basic syntax and classmates for advice
// TIP: I've created a file called `test` that will run some scripted tests and add processes in a quick succession. 
// =======================================================================

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Define semaphore variables
int mutex, full, empty;

// Define shared memory variables
int shmEndId, *endSig;

// Function prototypes
void readResources();
bool isInitialized();

int main() {
    // Read system resources
    readResources();    
   
    // Set end flag to 1 to signal completion
    endSig[0] = 1;

    // Release mutex and allow consumer to continue
    v(0, mutex);  
    v(0, full);

    // Detach shared memory
    if (shmdt(endSig) == -1 ) 
        printf("shmgm: ERROR in detaching.\n");
   
    return 0;
}

// Function to read resources from a file
void readResources() {
    // Check if consumer instance is initialized
    if (!isInitialized()) {
        printf("No Consumer instance is running.\n");
        exit(1);
    }

    // Open file for reading
    FILE *fp;
    fp = fopen("resources.txt", "r");
    if (fp == NULL) {
        printf("Error reading file.\n");
        exit(1);
    }

    int i;
    // Read from file and assign values
    for(i=0; i<5; i++) {
        fscanf(fp, "%d\n", &shmEndId); 
    }

    // Attach shared memory
    if ((endSig = (int*) shmat(shmEndId, NULL, SHM_RND)) == (void *)-1) {
        printf("Failed to attach shared memory.\n");
    }
       
    fscanf(fp, "%d\n", &mutex);
    fscanf(fp, "%d\n", &full);
    fclose(fp);
}

// Function to check if resources are initialized
bool isInitialized() {
    FILE *fp;
    fp = fopen("resources.txt", "r");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}

// Semaphore operation: P
p(int s, int sem_id) {
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    if((semop(sem_id, &sops, 1)) == -1)
        printf("%s", "'P' error\n");
}

// Semaphore operation: V
v(int s, int sem_id) {
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    if((semop(sem_id, &sops, 1)) == -1)
        printf("%s","'V' error\n");
}
