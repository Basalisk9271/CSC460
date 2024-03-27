// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Bob's Deadly Diner
// Due Date: Mar 5, 2024
// Sources: Chat-GPT for basic syntax and classmates for advice
// =======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>

#define NUM_PHILOSOPHERS 5

sem_t chopsticks[NUM_PHILOSOPHERS];
int sem_id; // Declare sem_id globally

// Define semaphore operations
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

void Think(int id) {
    int i, tabs;
    for (tabs = 0; tabs < id; tabs++) {
        putchar('\t'); // Print tabs
    }
    printf("%*d THINKING\n", id+1, id);
    // Simulate thinking by wasting some CPU cycles
    for (i = 0; i < 100000; i++);
}

void GetHungry(int id) {
    int i, tabs;
    for (tabs = 0; tabs < id; tabs++) {
        putchar('\t'); // Print tabs
    }
    printf("%*d HUNGRY\n", id+1, id);
    p(id, sem_id); // Pick up left chopstick
    // Simulate a delay between picking up left and right chopsticks
    for (i = 0; i < 100000; i++);
    p((id + 1) % NUM_PHILOSOPHERS, sem_id); // Pick up right chopstick
}

void Eat(int id) {
    int i, tabs;
    for (tabs = 0; tabs < id; tabs++) {
        putchar('\t'); // Print tabs
    }
    printf("%*d EATING\n", id + 1, id);
    // Simulate eating by wasting some CPU cycles
    for (i = 0; i < 100000; i++);

    v(id, sem_id); // Put down left chopstick
    v((id + 1) % NUM_PHILOSOPHERS, sem_id); // Put down right chopstick
}

void philosopher(int id) {
    while (1) {
        Think(id);
        GetHungry(id);
        Eat(id);
    }
}

int main() {
    int i;

    // Create semaphore set
    if ((sem_id = semget(IPC_PRIVATE, NUM_PHILOSOPHERS, IPC_CREAT | 0777)) == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (semctl(sem_id, i, SETVAL, 1) == -1) {
            perror("semctl SETVAL");
            exit(EXIT_FAILURE);
        }
    }

    // Create philosopher processes
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pid_t pid = fork();

        if (pid == 0) { // Child process
            philosopher(i);
            exit(0);
        } else if (pid < 0) { // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all child processes to finish (will never be reached in the deadlock scenario)
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        wait(NULL);
    }

    // Destroy semaphores
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
        exit(EXIT_FAILURE);
    }

    return 0;
}
