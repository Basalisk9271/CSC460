// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Due Date: Mar 20, 2024
// Sources: Chat-GPT for basic syntax and classmates for advice
// =======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define SHM_KEY 9876
#define SEM_KEY 1234

// Semaphore functions
void p(int s, int sem_id);
void v(int s, int sem_id);

// Function prototypes
void initSystem();
// void depositWithdraw(int processID, int amount, int numIterations);
void cleanup();

// Global variables
int *coins = NULL;
int semID = -1;
int shmID = -1;

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc == 1) {
        // No arguments provided
        initSystem();
    } else if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        // Cleanup request
        cleanup();
    } else if (argc == 2) {
        // Simulation request with integer argument
        int numIterations = atoi(argv[1]);


        if (numIterations < 1 || numIterations > 100) {
            fprintf(stderr, "Error: Number of iterations must be between 1 and 100.\n");
            exit(EXIT_FAILURE);
        }

        // Read values from "cryptodata" file
        FILE *file = fopen("cryptodata", "r");
        if (file == NULL) {
            fprintf(stderr, "Error: Unable to open cryptodata file.\n");
            exit(EXIT_FAILURE);
        }

        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "SHM_ID:") != NULL) {
                sscanf(line, "SHM_ID: %d", &shmID);
            } else if (strstr(line, "SEM_ID:") != NULL) {
                sscanf(line, "SEM_ID: %d", &semID);
            }
        }

        fclose(file);

        if (shmID == -1 || semID == -1) {
            fprintf(stderr, "Error: SHM_ID or SEM_ID not found in cryptodata file.\n");
            exit(EXIT_FAILURE);
        }

        // Attach shared memory
        coins = (int*)shmat(shmID, NULL, 0);
        if (coins == (int*)(-1)) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        // Debugging output
        printf("Pointer to coins: %p\n", (void *)coins);  // Print pointer address
        printf("Retrieved coin amount: %d\n", *coins);    // Print retrieved coin amount

        if (coins == NULL) {
            fprintf(stderr, "Error: System not initialized. Please run without arguments first.\n");
            exit(EXIT_FAILURE);
        }

        printf("Retrieved coin amount: %d\n", *coins); // Print retrieved coin amount for testing
        int i, j, value, amount;
        
         // Initialize process pool for parent processes
        for (value = 0; value < 16; value++) {
            amount = 1 << value;
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process created by each parent
                pid_t childPid = fork();
                if (childPid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (childPid == 0) {
                    // Child process subtracting coins
                    for (j = 0; j < numIterations; j++) {
                        p(0, semID);
                        *coins -= amount;
                        printf("\t\t%d: %d - %d = %d\n", getpid(), *coins + amount, amount, *coins);
                        v(0, semID);
                    }
                    exit(EXIT_SUCCESS);  // Child exits after completing its task
                } else {
                    // Parent process (child of initial parent) adds coins
                   for (j = 0; j < numIterations; j++) {
                    p(0, semID);
                    *coins += amount;
                    printf("%d: %d + %d = %d\n", getpid(), *coins - amount, amount, *coins);
                    v(0, semID);
                    }
                    exit(EXIT_SUCCESS);  // Parent exits after completing its task
                }
        }
        }

        // Wait for all child processes to finish
        for (i = 0; i < 16; i++) {
            wait(NULL);
        }

        return 0;
    } else {
        fprintf(stderr, "Usage: %s [numIterations | cleanup]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}

void initSystem() {
    // Check if previous instance is initialized
    FILE* file = fopen("cryptodata", "r");
    if (file != NULL) {
        // Read values from "cryptodata" file
        int savedShmID = -1, savedSemID = -1;
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "SHM_ID:") != NULL) {
                sscanf(line, "SHM_ID: %d", &savedShmID);
            } else if (strstr(line, "SEM_ID:") != NULL) {
                sscanf(line, "SEM_ID: %d", &savedSemID);
            }
        }

        fclose(file);

        if (savedShmID != -1 && savedSemID != -1) {
            // Attach shared memory to get the current coin amount
            int* savedCoins = (int*)shmat(savedShmID, NULL, 0);
            if (savedCoins == (int*)(-1)) {
                perror("shmat");
                exit(EXIT_FAILURE);
            }

            printf("Previous instance found with %d coins.\n", *savedCoins);
            exit(EXIT_SUCCESS);  // Exit successfully as no need to reinitialize
        }
    }

    // Create shared memory for coins
    int shmID = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0777);
    if (shmID == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory
    coins = (int*)shmat(shmID, NULL, 0);
    if (coins == (int*)(-1)) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize coins to 460
    *coins = 460;

    // Create semaphore
    semID = semget(SEM_KEY, 1, IPC_CREAT | 0777);
    if (semID == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphore value to 1
    semctl(semID, 0, SETVAL, 1);

    // Save IDs to file
    file = fopen("cryptodata", "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "SHM_ID: %d\nSEM_ID: %d\n", shmID, semID);
    fclose(file);

    printf("System initialized with %d coins.\n", *coins);
}


void cleanup() {
    // Read values from "cryptodata" file
    FILE *file = fopen("cryptodata", "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open cryptodata file.\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "SHM_ID:") != NULL) {
            sscanf(line, "SHM_ID: %d", &shmID);
        } else if (strstr(line, "SEM_ID:") != NULL) {
            sscanf(line, "SEM_ID: %d", &semID);
        }
    }

    fclose(file);

    if (shmID == -1 || semID == -1) {
        fprintf(stderr, "Error: SHM_ID or SEM_ID not found in cryptodata file.\n");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory
    coins = (int *)shmat(shmID, NULL, 0);
    if (coins == (int *)(-1)) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Print current coin count
    printf("Current coin count: %d\n", *coins);

    // Remove semaphore
    if (semctl(semID, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    // Detach from shared memory
    if (shmdt(coins) == -1) {
        perror("shmdt");
    }

    // Remove shared memory segment
    if (shmctl(shmID, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    // Delete "cryptodata" file
    if (remove("cryptodata") != 0) {
        perror("remove");
    }

    printf("Cleanup completed.\n");
}


void p(int s, int sem_id) {
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void v(int s, int sem_id) {
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}
