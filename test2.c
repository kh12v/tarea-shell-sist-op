#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


char** parse_command(char *cmd) {
    char** argv = malloc(64 * sizeof(char*));
    char* token;
    int i = 0;

    token = strtok(cmd, " \t\n");
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[i] = NULL;
    return argv;
}

// cat texto.txt | grep que | paste >> texto2.txt

int main() {
    char input[1024];

    while (1) {
        printf("Comando: ");
        if (!fgets(input, sizeof(input), stdin)) break;

        if (strncmp(input, "exit", 4) == 0) break;

        // Separating pipes
        char* commands[64];
        int ncmds = 0;
        char* token = strtok(input, "|");
        while (token != NULL) {
            commands[ncmds++] = token;
            token = strtok(NULL, "|");
        }

        // Creating pipes
        int pipes[2 * (ncmds - 1)];
        for (int i = 0; i < ncmds - 1; i++) {
            if (pipe(pipes + i*2) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        // Executing commands
        for (int i = 0; i < ncmds; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0) {
                    if (dup2(pipes[(i-1)*2], STDIN_FILENO) == -1) {
                        perror("dup2 stdin");
                        exit(1);
                    }
                }
                
                if (i < ncmds - 1) {
                    if (dup2(pipes[i*2 + 1], STDOUT_FILENO) == -1) {
                        perror("dup2 stdout");
                        exit(1);
                    }
                }

                // Closing pipes
                for (int j = 0; j < 2*(ncmds-1); j++) {
                    close(pipes[j]);
                }

                char** argv = parse_command(commands[i]);

                int redirect_append = 0;
                char* outfile = NULL;

                for (int k = 0; argv[k]; k++) {
                    if (strcmp(argv[k], ">") == 0 || strcmp(argv[k], ">>") == 0) {
                        redirect_append = (strcmp(argv[k], ">>") == 0);
                        outfile = argv[k+1];
                        argv[k] = NULL;
                        break;
                    }
                }

                if (outfile) {
                    int fd;
                    if (redirect_append)
                        fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    else
                        fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    if (fd < 0) {
                        perror("open");
                        exit(1);
                    }
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2 redir");
                        exit(1);
                    }
                    close(fd);
                }

                execvp(argv[0], argv);
                perror("execvp");
                exit(1);
            }
        }

        // Closing pipes
        for (int i = 0; i < 2*(ncmds-1); i++) {
            close(pipes[i]);
        }

        // Waiting children
        for (int i = 0; i < ncmds; i++) {
            wait(NULL);
        }
    }

    return 0;
}
