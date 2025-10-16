
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define NUM_CHILDREN 3

pid_t children[NUM_CHILDREN]; // store child PIDs

volatile sig_atomic_t stop_flag = 0;
volatile sig_atomic_t restart_flag = 0;
volatile sig_atomic_t terminate_flag = 0;

// Signal handler for stopping children
void handle_stop(int sig) {
    stop_flag = 1;
    printf("\n[Controller] Stop signal received.\n");
}

// Signal handler for restarting children
void handle_restart(int sig) {
    restart_flag = 1;
    printf("\n[Controller] Restart signal received.\n");
}

// Signal handler for termination
void handle_terminate(int sig) {
    terminate_flag = 1;
    printf("\n[Controller] Terminate signal received.\n");
}

// Function to spawn child processes
void spawn_children() {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            // Child process: simulate some work
            while (1) {
                printf("[Child %d] Working...\n", getpid());
                sleep(2);
            }
            exit(0);
        } else {
            // Parent stores child PID
            children[i] = pid;
        }
    }
}

// Function to stop children gracefully
void stop_children() {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(children[i], SIGSTOP); // stop child
        printf("[Controller] Stopped child %d\n", children[i]);
    }
}

// Function to resume children
void resume_children() {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(children[i], SIGCONT); // continue child
        printf("[Controller] Resumed child %d\n", children[i]);
    }
}

// Function to terminate children
void terminate_children() {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(children[i], SIGTERM);
        printf("[Controller] Terminated child %d\n", children[i]);
    }

    // Wait for children to exit
    for (int i = 0; i < NUM_CHILDREN; i++) {
        waitpid(children[i], NULL, 0);
    }
}

int main() {
    printf("[Controller] Starting job controller...\n");

    // Register signal handlers
    signal(SIGUSR1, handle_stop);      // Stop children
    signal(SIGUSR2, handle_restart);   // Restart children
    signal(SIGINT, handle_terminate);  // Terminate controller

    spawn_children();

    // Controller main loop
    while (1) {
        sleep(1);

        if (stop_flag) {
            stop_children();
            stop_flag = 0;
        }

        if (restart_flag) {
            resume_children();
            restart_flag = 0;
        }

        if (terminate_flag) {
            terminate_children();
            printf("[Controller] Exiting gracefully.\n");
            exit(0);
        }
    }

    return 0;
}
