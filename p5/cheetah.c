// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Project 5 - Cheetah Sync
// Due Date: Mar 1, 2024
// Sources: Chat-GPT for basic syntax, classmates for advice, and yessync2.c 
//  from Dr. Bob's examples as a guide
// =======================================================================

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>

main(int argc, char *argv[])
{
int i, sem_id, N;               // Holds the number of procs to generate
int semArrIndex = 0;           // used to identify procs in sync
char letter = 'A';

/*****  Get the command line argument for how many procs. ****/
if (argc != 2) {
    printf("Usage: %s <N>\n", argv[0]);
    printf("N should be between 1 and 26 inclusive.\n");
    return 1;
}

N = atoi(argv[1]);

if (N < 1 || N > 26) {
    printf("N should be between 1 and 26 inclusive.\n");
    return 1;
}

/*****  Make a note of "who" is the first Process *****/

int firstID = getpid();

//  Ask OS for semaphores.
sem_id = semget (IPC_PRIVATE, N, 0777);

//  See if you got the request.
if (sem_id == -1)
   {
    printf("%s","SemGet Failed.\n");
    return (-1);
   }

// Set the smes to 1 0 0 0 ...
semctl (sem_id, 0, SETVAL, 1);

for (i = 1;i < N; i++){
    semctl(sem_id, i, SETVAL, 0);
}

/*************  Spawn all the Processes *********/

for (i = 1; i < N; i++)
{
  if (fork() > 0) break; // send parent on to Body
  semArrIndex++;
}

/*************  BODY  OF  PROGRAM     ***********/

for (i = 0; i < N; i++)
{
    p(semArrIndex, sem_id);

    printf("%c: %d\n",letter+semArrIndex, getpid());
    v(((semArrIndex + 1) % N), sem_id);
}


/****   Cclean-up the Semaphores  ******/
sleep(1);
if (firstID == getpid())         // ONLY need one process to do this
if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
    printf("ERROR removing Semaphore.\n");
// printf("alldone\n");
}

p(int s,int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = -1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}

v(int s, int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = 1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s","'V' error\n");
}

