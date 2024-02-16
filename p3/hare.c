// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Project 3 - Hare Sync
// Due Date: Feb 21, 2024
// Sources: Chat-GPT for basic syntax, classmates for advice, and yessync2.c 
//  from Dr. Bob's examples as a guide
// =======================================================================

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define TURN shmem[0]

main(int argc, char *argv[])
{
int i, shmid, *shmem;
int N;                  // Holds the number of procs to generate
int myID = 0;           // used to identify procs in sync
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


/*****  Get Shared Memory and ID *****/

shmid  =  shmget(IPC_PRIVATE, sizeof(int), 0770);
if (shmid == -1)
    {
    printf("Could not get shared memory.\n");
    return(0);
    }


/****   Attach to the shared memory  ******/

shmem = (int *) shmat(shmid, NULL, SHM_RND);


/****  Initialize teh shared memory ****/

TURN = 0;


/*************  Spawn all the Processes *********/

for (i = 1; i < N; i++)
{
  if (fork() > 0) break; // send parent on to Body
  myID++;
}
// printf("I'm Alive: %d - %d\n",getpid(),myID);
sleep(1);


/*************  BODY  OF  PROGRAM     ***********/

for (i = 0; i < N; i++)
{
    while(TURN != myID);  /** busy wait for Child **/

    printf("%c: %d\n",letter+TURN, getpid());
    TURN = (TURN + 1) % N;
}


/****   Detach and clean-up the shared memory  ******/

if (shmdt(shmem) == -1 ) printf("shmgm: ERROR in detaching.\n");

sleep(1);

if (firstID == getpid())         // ONLY need one process to do this
if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
    printf("ERROR removing shmem.\n");
// printf("alldone\n");
}

