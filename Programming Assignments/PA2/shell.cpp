#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>
//included for the open file access modes
#include <fcntl.h>
#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    //copy the file descriptors for both standard in and out
    int stdout2 = dup(STDOUT_FILENO);

    int stdin2 = dup (STDIN_FILENO);
    //will need a buffer and size variable
    char* buffer;
    int size;
    //previous and current directories will be needed
    char* previousDir;
    char* currentDir;
    //background process
    vector<int> process;
    //infinite loop statement
    for (;;) {
        // need date/time, username, and absolute path to current dir
        //get current time
        time_t time1 = time(0);
        //searches for the username
        char* user = getenv("USER");

        char* date = ctime(&time1);
        //size set to the  "maximum number of characters in a complete path name"
        size = pathconf(".", _PC_PATH_MAX);

        buffer = (char*) malloc(size);
        if(buffer != NULL){
            //getting the current directory
            currentDir = getcwd(buffer, size);
            if(currentDir != NULL){
                if(previousDir == NULL){
                    previousDir = currentDir;
                }
                cout << YELLOW << date << user << ":" << currentDir << "$" << NC << " ";
            }
            else
            {
                //not sure if we need specific error messages...
                perror("Problem with directory");
            }
        }
        //finding out what the user wants to do
        string in;
        getline(cin, in);

        if (in == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(in);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        //uncommented from code
        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }
        //iterating through the tokens
        for(long unsigned int i = 0; i < tknr.commands.size(); i++)
        {
            //file descriptors that will be used with my pipes
            int file[2];
            //checking for errors in the pipe
            if(pipe(file) == -1){
                perror("Problem with the pipe");
                exit(EXIT_FAILURE);

            }
            //storing the pid from the fork
            pid_t pNum = fork();
            if(pNum < 0){
                perror("Problem with the fork");
                exit(EXIT_FAILURE);
            }
            //getting the current command at the specified index
            Command* command = tknr.commands.at(i);
            for(long unsigned int j = 0; j < process.size(); j++){
                int x = 0;
                pid_t pNum = process[j];
                //checking for errors with the process
                if(waitpid(pNum,&x, WNOHANG) != 0){
                    process.erase(process.begin() + j);
                    x--;
                }
                else{
                    continue;
                }
            }
            //checking for child process
            if(pNum == 0){
                if( tknr.commands.size() -1 == i){
                    dup2(stdout2, file[1]);
                }
                else
                {
                    //write end of the pipe is now STDOUT
                    dup2(file[1], STDOUT_FILENO);
                }
                //setting a new args variable
                vector<string> args = command->args;
                char** args2 = new char*[args.size() + 1];
                for(long unsigned int i = 0; i < args.size(); i++){
                    args2[i] = (char*) args[i].c_str();
                }
                //terminating character
                args2[args.size()] = nullptr;
                //checking if the command has an input or output
                if(command->hasInput()){
                    int file2 = open(command->in_file.c_str(), O_RDONLY);
                    dup2(file2, STDIN_FILENO);
                }
                if(command->hasOutput()){
                    //setting the flags that denote the different aspects of the file
                    int file3 = open(command->out_file.c_str(), S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP | O_CREAT | O_WRONLY | O_TRUNC);
                    dup2(file3, STDOUT_FILENO);
                }
                //working with the cd command
                string command2 = args2[0];
                if(command2 == "cd"){
                    string first = args2[1];
                    if(first == "-"){
                        if(currentDir == previousDir){
                            cerr << "Problem with directories" << endl;
                        }
                        else
                        {
                            chdir(previousDir);
                            previousDir = currentDir;
                        }
                    }
                    else{
                        previousDir = currentDir;
                        chdir(args2[1]);
                    }
                }
                //checking if execvp can run with no problems
                else if(execvp(args2[0], args2) < 0){
                    perror("Problem with execvp");
                    exit(EXIT_FAILURE);
                }
                delete[] args2;
            }
            else{
                int z = 0;
                //closing write end
                close(file[1]);
                //read end of the pipe is now STDIN
                dup2(file[0], STDIN_FILENO);
                //checking if the command is in the background
                if(command->isBackground()){
                    if(waitpid(pNum, &z, WNOHANG) == 0){
                        process.push_back(pNum);
                    }
                }
                else{
                    waitpid(pNum, &z, 0);
                    if(z > 1){
                        exit(z);
                    }
                }
            }
        }
        //repopulating the file descriptors that include the standard in and out
        dup2(stdout2, STDOUT_FILENO);
        dup2(stdin2, STDIN_FILENO);
    }
    //closing out the new file descriptors I made in the beginning as they are no longer needed
    close(stdout2);
    close(stdin2);
    //do not need buffer anymore
    free(buffer);
    buffer = NULL;
}
