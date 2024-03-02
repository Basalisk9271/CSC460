// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Project 4 - Grocery List
// Due Date: Mar 1, 2024
// Sources: Chat-GPT for basic syntax, classmates for advice, and yessync2.c 
//  from Dr. Bob's examples as a guide
// =======================================================================

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>

struct bbStruct
{
        int id;
        char name[20];
        int favNum;
        char favFood[30];
};

main(int argc, char *argv[])
{
int shmid,i;
struct bbStruct *shmem;
FILE *fopen(), *fp;


/*****  Open File to hold Shared Memory ID  *****/

if((fp = fopen("/pub/csc460/bb/BBID.txt","r")) == NULL)
    {
    printf("watch:  could not open file.\n");
    return(0);
    }


/*****  Get Shared Memory ID and close file  *****/

    fscanf(fp,"%d",&shmid);
    fclose(fp);

/****   Attach to the shared memory  ******/

shmem = (struct bbStruct *) shmat(shmid, NULL, SHM_RND);

printf("\n\nHERE IS OUR SHARED GROCERY LIST:\n\n");
for (i=0;i<22;i++)
    {
       printf("%2d: %20s| %8d| %30s|\n",shmem->id,shmem->name,shmem->favNum,shmem->favFood);
       shmem++;
    }
printf("\n\n");


}
