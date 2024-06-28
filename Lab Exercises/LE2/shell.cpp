/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <stdio.h> // perror
#include <stdlib.h> // exit, EXIT_FAILURE
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <string.h> // memcpy
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using namespace std;

int main (int argc, char **argv) {
    char p_name[256];
    if (argc != 2) {        // Add the correct condition; hint: what should the value of argc be?
        fprintf(stderr, "Incorrect format. Use ./shell <process name>\n");
        exit(EXIT_FAILURE);
    }

    // copy argv process name into p_name; copy memory instead of strcpy
    memcpy(p_name, argv[1], sizeof(argv[1]));
    // lists all the processes
    // create a pointer to an array for execvp, say cmd1
    char* cmd1[] = {(char*)"ps",(char*)"aux", NULL};
    // grep for the input
    // create a pointer to an array for execvp, say cmd2
    char* cmd2[] = {(char*) "grep", p_name, NULL};

    // create pipe before forking
    int fd[2];
    if (pipe(fd) == -1){
        perror("Cannot create a pipe");
        exit(EXIT_FAILURE);
    }
    // pipe one command to another
    // start by forking
    pid_t x = fork();
    if(x == -1){
        perror("Could not perform the first fork");
        exit(EXIT_FAILURE);
    }
    // if it is a child, fork again to run the commands
    if(x == 0){
        //close read end of pipe
        close(fd[0]);
        //redirect standard out to write end of pipe
        dup2(fd[1], STDOUT_FILENO);
        //execute ps command
        execvp(cmd1[0], cmd1);

        //if execvp works fine, this will not run
        perror("First execvp did not work");
        exit(EXIT_FAILURE);
    }
    // now you have the current process, its child, and grandchild process
    // which of the child or the grandchild exec cmd1? the other runs cmd2
    // The output of which command act as the input of the other?
    // Use dup2() to redirect outputs and inputs
    // Make sure to close the unused sides with close(fd[0]) or close(fd[1])
    pid_t y = fork();
    if(y == -1){
        perror("Could not perform the second fork");
        exit(EXIT_FAILURE);
    }

    if(y == 0){
        //close write end of pipe
        close(fd[1]);
        //redirect standard in to the read end of the pipe
        dup2(fd[0], STDIN_FILENO);
        //execute the grep command
        execvp(cmd2[0], cmd2);

        //if execvp works fine, this will not run
        perror("Could not perform the second execvp");
        exit(EXIT_FAILURE);
    }
    //close pipe on the side of the parent process
    close(fd[1]);
    close(fd[0]);
    //wait for both child processes to be done
    waitpid(x, nullptr, 0);
    waitpid(y, nullptr, 0);
    return 0;
}
