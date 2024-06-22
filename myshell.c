#define _GNU_SOURCE // Ensure strdup is available
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define MAX_HISTORY 100

// Array to store command history
char *history[MAX_HISTORY];
int history_count = 0;

// Function to add a command to the history
void add_to_history(char *command)
{
    // If there's space in the history array, add the command
    if (history_count < MAX_HISTORY)
    {
        history[history_count] = strdup(command);
        history_count++;
    }
    else
    {
        // If history is full, remove the oldest command and add the new one
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY - 1] = strdup(command);
    }
}

// Function to print the command history
void print_history()
{
    for (int i = history_count - 1; i >= 0; i--)
    {
        printf("%d %s", history_count - i, history[i]);
    }
}

int main(void)
{
    // Redirect stderr to stdout
    close(2);
    dup(1);
    char command[BUFFER_SIZE];

    while (1)
    {
        // Print the shell prompt
        fprintf(stdout, "my-shell> ");

        // Clear the command buffer
        memset(command, 0, BUFFER_SIZE);

        // Read the user input
        fgets(command, BUFFER_SIZE, stdin);

        // Check for the "exit" command
        if (strncmp(command, "exit", 4) == 0)
        {
            break;
        }

        // Check for the "history" command
        if (strncmp(command, "history", 7) == 0)
        {
            add_to_history(command);
            print_history();
            continue;
        }

        // Add the command to the history
        add_to_history(command);

        // Check if the command should run in the background
        int background = 0;
        if (command[strlen(command) - 2] == '&')
        {
            background = 1;
            command[strlen(command) - 2] = '\0'; // Remove the '&' character
        }

        // Tokenize the command input into arguments
        char *args[BUFFER_SIZE];
        char *token = strtok(command, " \t\n");
        int i = 0;
        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[i] = NULL;

        // Fork a new process
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process: Execute the command
            if (execvp(args[0], args) == -1)
            {
                perror("error");
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            // Fork failed
            perror("error");
        }
        else
        {
            // Parent process: Wait for the child if not running in background
            if (!background)
            {
                waitpid(pid, NULL, 0);
            }
        }
    }

    // Free the history array before exiting
    for (int i = 0; i < history_count; i++)
    {
        free(history[i]);
    }

    return 0;
}
