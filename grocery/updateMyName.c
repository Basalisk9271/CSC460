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
#include <string.h>

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
    char NewName[20];
    struct bbStruct *shmem;
    FILE *fopen(), *fp;

    /*****  Get the command line argument for how many procs. ****/
    if (argc != 2) {
        printf("Usage: %s <NewName>\n", argv[0]);
        return 1;
    }

    // Copy the string from argv[1] into NewName
    strncpy(NewName, argv[1], sizeof(NewName) - 1);
    NewName[sizeof(NewName) - 1] = '\0';  // Ensure null-termination

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
    shmem+=5;

    strcpy(shmem->name,NewName);

}