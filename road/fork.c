// =======================================================================
// Name: Gabe Imlay
// Class: Operating Systems
// Assignment: Project 1 - Fork in the Road
// Due Date: Feb 1, 2024
// Sources: Chat-GPT for basic syntax and classmates for advice
// =======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to print the header for process information
void print_info_header() {
    printf("Gen\tPID\tPPID\n");
}

// Function to print process information
void print_info(int generation, pid_t pid, pid_t ppid) {
    printf("%d\t%d\t%d\n", generation, pid, ppid);
}

// Recursive function to create a family tree of processes
void create_family_tree(int N, int generation) {
    // Validate input range
    if (N < 0 || N > 5) {
        fprintf(stderr, "Error: N should be between 0 and 5 (inclusive)\n");
        exit(EXIT_FAILURE);
    }

    // Variable to track the maximum generation
    static int max_generation = 0;

    // Update the maximum generation if needed
    if (generation > max_generation) {
        max_generation = generation;
    }

    // Print information for the initial (0th) generation
    if (generation == 0) {
        print_info_header();
        print_info(generation, getpid(), getppid());
    }

    // Create child processes for the current generation
    if (generation < N) {
        pid_t pids[N - generation];
        int i;

        for (i = 0; i < N - generation; i++) {
            pid_t pid = fork();

            if (pid < 0) {
                fprintf(stderr, "Error creating child process\n");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                // Child process
                print_info(generation + 1, getpid(), getppid());
                create_family_tree(N, generation + 1);  // Recursive call for child processes
                exit(EXIT_SUCCESS);
            } else {
                // Parent process
                pids[i] = pid;  // Store child PID for waiting later
            }
        }

        // Wait for all child processes to finish before moving to the next generation
        for (i = 0; i < N - generation; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

int main(int argc, char *argv[]) {
    // Check for the correct number of command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Convert command line argument to an integer
    int N = atoi(argv[1]);

    // Start creating the family tree with the initial (0th) generation
    create_family_tree(N, 0);

    return 0;
}
