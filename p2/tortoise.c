// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Project 2 - Tortoise Synch
// Due Date: Feb 14, 2024
// Sources: Chat-GPT for basic syntax and classmates for advice
// =======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        printf("N should be between 1 and 26 inclusive.\n");
        return 1;
    }

    int N = atoi(argv[1]);

    if (N < 1 || N > 26) {
        printf("N should be between 1 and 26 inclusive.\n");
        return 1;
    }

    if ((fp = fopen("syncfile", "w")) == NULL) {
        printf(":( could not open syncfile to write.\n");
        return 0;
    }

    fprintf(fp, "%d", 0);
    fclose(fp);

    int i, j, myID, otherID;
    char letter = 'A';
    int found;

    for (i = 0; i < N; i++) {
        if (fork() == 0) {
            myID = i;
            otherID = (i + 1) % N;

            for (j = 0; j < 5; j++) {
                found = -1;
                while (found != myID) {
                    if ((fp = fopen("syncfile", "r")) == NULL) {
                        printf(":( could not open syncfile to read.\n");
                        return 0;
                    }

                    fscanf(fp, "%d", &found);
                    fclose(fp);
                }

                printf("%c:%d\n", letter + i, getpid());

                if ((fp = fopen("syncfile", "w")) == NULL) {
                    printf(":( could not open syncfile to write.\n");
                    return 0;
                }

                fprintf(fp, "%d", otherID);
                fclose(fp);
            }
            break;
        }
    }
// Wait for all child processes to finish
    for (i = 0; i < N; i++) {
        wait(NULL);
    }


    return 0;
}
