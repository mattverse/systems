#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "shell.h"

// Function to parse user input into command and arguments
void command_line_parse(char *input, char *command, char **arguments) {
    char *token = strtok(input, " ");
    strcpy(command, token);

    int arg_count = 0;
    while (token != NULL) {
        arguments[arg_count] = token;
        arg_count++;

        token = strtok(NULL, " ");
    }
    arguments[arg_count] = NULL;
}

void command_line_execute(char *command, char **arguments) {
    if (execvp(command, arguments) == -1) {
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char input[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char *arguments[MAX_ARGUMENTS];

    while (1) {
        printf("SHELL> ");
        fflush(stdout);

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // String compare user input with "exit". If user input "exit",
        // gracefullt exit program.
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Parse the input into command and arguments
        command_line_parse(input, command, arguments);

        int pipes[2]; // Used for pipe if needed

        // Check for input/output redirection
        int input_fd = 0; // Default to stdin
        int output_fd = 1; // Default to stdout

        for (int i = 0; arguments[i] != NULL; i++) {
            if (strcmp(arguments[i], "<") == 0) {
		        printf("Input Redirection Detected!\n");
                // Input redirection
                arguments[i] = NULL; // Remove "<" from the argument list
                input_fd = open(arguments[i + 1], O_RDONLY);
                if (input_fd == -1) {
                    perror("Input redirection failed");
                    exit(EXIT_FAILURE);
                }
                i++;
            } else if (strcmp(arguments[i], ">") == 0) {
		        printf("Output Redirection Detected!\n");
                // Output redirection
                arguments[i] = NULL; // Remove ">" from the argument list
                output_fd = open(arguments[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (output_fd == -1) {
                    perror("Output redirection failed");
                    exit(EXIT_FAILURE);
                }
                i++;
            } else if (strcmp(arguments[i], "|") == 0) {
		        printf("Pipe Detected!\n");
                // Pipe
                arguments[i] = NULL; // Split the command into two parts
                if (pipe(pipes) == -1) {
                    perror("Pipe creation failed");
                    exit(EXIT_FAILURE);
                }

                // Fork a child process for the left side of the pipe
                pid_t left_pid = fork();
                if (left_pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } else if (left_pid == 0) { // check if this is a child process.
                    close(pipes[0]); // Close read end of pipe since it is not needed
                    dup2(pipes[1], 1); // Redirect the output of the left child to the write end.
                    close(pipes[1]); // Close the write end of the pipe
                    command_line_execute(command, arguments); // Execute the left side of the pipe
                }

                // Fork another child process for the right side of the pipe
                pid_t right_pid = fork();
                if (right_pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } else if (right_pid == 0) {
                    close(pipes[1]); // Close the write end  since it is not needed
                    dup2(pipes[0], 0); // Redirect the output to the read end of the pipe.
                    close(pipes[0]); // Close the read end of the pipe
                    char *right_command = arguments[i + 1];
                    char **right_arguments = &arguments[i + 1];
                    // command itself should be the first argument
                    right_arguments[0] = right_command; 
                    command_line_execute(right_command, right_arguments);
                } else {
                     // Parent process
                    close(pipes[0]);
                    close(pipes[1]);
                }
            
                int status;
                waitpid(left_pid, &status, 0); // Wait for the left child process
                waitpid(right_pid, &status, 0); // Wait for the right child process

                // Continue to the next iteration of the loop
                continue;
            }
        }

        // Fork a child process to execute the command
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            // Redirect input if needed
            if (input_fd != 0) {
                dup2(input_fd, 0);
                close(input_fd);
            }

            // Redirect output if needed
            if (output_fd != 1) {
                dup2(output_fd, 1);
                close(output_fd);
            }
            command_line_execute(command, arguments);
        } else {
            // Parent process
	        // Wait child process and check if it is exited or terminated
            int status;
            waitpid(pid, &status, 0); // Wait for the child process
            if (WIFEXITED(status)) {
                printf("Child process exited with status %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process terminated by signal %d\n", WTERMSIG(status));
            }
        }
    }

    return 0;
}

