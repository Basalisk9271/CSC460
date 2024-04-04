#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>

#define NUM_PHILOSOPHERS 5
#define MAX_THINK_TIME 7 // Maximum seconds for thinking
#define MAX_EAT_TIME 3   // Maximum seconds for eating

enum PhilosopherState
{
    THINKING,
    HUNGRY,
    EATING,
    DEAD
};

struct PhilosopherInfo
{
    enum PhilosopherState state;
};

struct PhilosopherInfo *philosophers;

int sem_id;
int *philosophers_died;

void p(int s, int sem_id)
{
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1)
    {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void v(int s, int sem_id)
{
    struct sembuf sops;
    sops.sem_num = s;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1)
    {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

void Think(int id)
{
    srand(time(0)+id);
    philosophers[id].state = THINKING;
    sleep(rand() % (10 - 4 + 1) + 4); // Random think time between 4-10 seconds
}

void GetHungry(int id)
{
    philosophers[id].state = HUNGRY;
    int left = id;                        // Index of the left chopstick
    int right = (id + 1) % NUM_PHILOSOPHERS; // Index of the right chopstick

    p(left, sem_id);   // Pick up left chopstick
    philosophers[id].state = HUNGRY; // Set state to hungry after acquiring left chopstick

    if (semctl(sem_id, right, GETVAL, 0) == 1)
    {
        p(right, sem_id);  // Pick up right chopstick
        philosophers[id].state = EATING; // Change state to eating after acquiring both chopsticks
        return;
    }

    v(left, sem_id); // Release left chopstick
    philosophers[id].state = HUNGRY; // Remain in hungry state
}

void Eat(int id)
{
    philosophers[id].state = EATING; // Set state to eating
    srand(time(0)+id);
    int eat_time = rand() % 3 + 1;
    sleep(eat_time); // Sleep for the eat time duration

    v(id, sem_id); // Put down left chopstick
    v((id + 1) % NUM_PHILOSOPHERS, sem_id); // Put down right chopstick

    philosophers[id].state = THINKING; // Change state to thinking after eating
}

void philosopher(int id)
{
    int elapsed_time = 0;
    time_t start_time = time(NULL);  // Record the start time

    while (elapsed_time < 60 && philosophers[id].state != DEAD)
    {
        if (philosophers[id].state == THINKING)
        {
            Think(id);
        }

        GetHungry(id);

        if (philosophers[id].state == EATING)
        {
            Eat(id);
        }

        time_t current_time = time(NULL);
        elapsed_time = current_time - start_time;  // Update elapsed time
    }

    philosophers[id].state = DEAD; // Set state to DEAD
    (*philosophers_died)++; // Increment the count of philosophers who have died
}

void printState(int time)
{
    printf("%2d. ", time); // Left-aligned with width 3 for the time
    int i;
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        if (philosophers[i].state == THINKING)
        {
            printf("%-12s", "Thinking"); // Left-aligned with width 8 for the state
        }
        else if (philosophers[i].state == HUNGRY)
        {
            printf("%-12s", "Hungry"); // Left-aligned with width 8 for the state
        }
        else if (philosophers[i].state == EATING)
        {
            printf("%-12s", "Eating"); // Left-aligned with width 8 for the state
        }
        else if (philosophers[i].state == DEAD)
        {
            printf("%-12s", "Dead"); // Left-aligned with width 8 for the state
        }
    }
    printf("\n");
}


int main()
{
    int i, shm_id, shm_id_dead;

    shm_id_dead = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);
    if (shm_id_dead == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory for philosophers_died
    philosophers_died = (int *)shmat(shm_id_dead, NULL, 0);
    if (philosophers_died == (int *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    *philosophers_died = 0; // Initialize philosophers_died to 0

    // Create shared memory segment for philosophers
    shm_id = shmget(IPC_PRIVATE, NUM_PHILOSOPHERS * sizeof(struct PhilosopherInfo), IPC_CREAT | 0777);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory for philosophers
    philosophers = (struct PhilosopherInfo *)shmat(shm_id, NULL, 0);
    if (philosophers == (struct PhilosopherInfo *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    sem_id = semget(IPC_PRIVATE, NUM_PHILOSOPHERS, IPC_CREAT | 0777);
    if (sem_id == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        if (semctl(sem_id, i, SETVAL, 1) == -1)
        {
            perror("semctl SETVAL");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Child process
            philosopher(i);
            exit(0);
        }
        else if (pid < 0)
        { // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    int time = 1;
    while (*philosophers_died < NUM_PHILOSOPHERS)
    {
        sleep(1);
        printState(time);
        // printf("%d\n", *philosophers_died);
        time++;
    }

    if (shmdt(philosophers) == -1 || shmdt(philosophers_died) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("semctl IPC_RMID");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1 || shmctl(shm_id_dead, IPC_RMID, NULL) == -1)
    {
        perror("shmctl IPC_RMID");
        exit(EXIT_FAILURE);
    }

    return 0;
}
