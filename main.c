#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>


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

void print_mem_peak(FILE *destino) {
    struct rusage uso;
    if (getrusage(RUSAGE_CHILDREN, &uso) == 0) {
        fprintf(destino, "Peak memoria residente: %ld KB\n", uso.ru_maxrss);
    }
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

                if(argv[0] && strcmp(argv[0], "miprof") == 0){
                    //seccion con myprof
                    if(argv[1] && strcmp(argv[1], "ejec") == 0){
                        //muestra a pantalla


                        int redirect_append = 0;
                        char* outfile = NULL;

                        for (int k = 2; argv[k]; k++) {
                            if (strcmp(argv[k], ">") == 0 || strcmp(argv[k], ">>") == 0) {
                                redirect_append = (strcmp(argv[k], ">>") == 0);
                                outfile = argv[k+1];
                                argv[k] = NULL;


                                break;
                            }
                        }

                        //construccion con sin simbolos
                        char *with_time[64];
                        int i = 0;
                        with_time[i] = "time";
                        i++;

                        for (int j = 2; argv[j] != NULL; j++) {
                            with_time[i] = argv[j];
                            i++;
                        }
                        with_time[i] = NULL;

                        //hijo ejecuta el comando y el padre mide memoria
                        pid_t cpid = fork();
                        if (cpid == 0) {
                            if (outfile) {
                                int fd;
                                if (redirect_append)
                                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                                else
                                    fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                                if (fd < 0) { perror("open"); exit(1); }
                                if (dup2(fd, STDOUT_FILENO) == -1) { perror("dup2 redir"); exit(1); }
                                close(fd);
                            }
                            execvp(with_time[0], with_time);
                            perror("execvp");
                            exit(1);
                        } else {
                            waitpid(cpid, NULL, 0);
                            print_mem_peak(stdout);
                        }

                    }
                    else if (argv[1] && strcmp(argv[1], "ejecsave") == 0){
                        //guarda en un archivo

                        int redirect_append = 0;
                        char* outfile = NULL;
                        FILE *archivo; 
                        char *nombre[] = argv[2];
                        archivo = fopen(nombre, "a");

                        for (int k = 3; argv[k]; k++) {
                            if (strcmp(argv[k], ">") == 0 || strcmp(argv[k], ">>") == 0) {
                                redirect_append = (strcmp(argv[k], ">>") == 0);
                                outfile = argv[k+1];
                                argv[k] = NULL;


                                break;
                            }
                        }

                        //construccion con sin simbolos
                        char *with_time[64];
                        int i = 0;
                        with_time[i] = "time";
                        i++;

                        for (int j = 3; argv[j] != NULL; j++) {
                            with_time[i] = argv[j];
                            i++;
                        }
                        with_time[i] = NULL;

                        //lo mismo que el anterior
                        pid_t cpid = fork();
                        if (cpid == 0) {
                            //almacena time
                            int fd_archivo = fileno(archivo);
                            if (dup2(fd_archivo, STDERR_FILENO) == -1) {
                                perror("dup2 stderr");
                                exit(1);
                            }
                            if (outfile) {
                                int fd;
                                if (redirect_append)
                                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                                else
                                    fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                                if (fd < 0) { perror("open"); exit(1); }
                                if (dup2(fd, STDOUT_FILENO) == -1) { perror("dup2 redir"); exit(1); }
                                close(fd);
                            }
                            execvp(with_time[0], with_time);
                            perror("execvp");
                            exit(1);
                        } else {
                            waitpid(cpid, NULL, 0);
                            // Peak al archivo
                            print_mem_peak(archivo);
                            fclose(archivo);
                        }

                    }
                    else if (argv[1] && strcmp(argv[1], "ejecutar") == 0){
                        //ejecucion con tiempo limite

                        int redirect_append = 0;
                        char* outfile = NULL;

                        for (int k = 3; argv[k]; k++) {
                            if (strcmp(argv[k], ">") == 0 || strcmp(argv[k], ">>") == 0) {
                                redirect_append = (strcmp(argv[k], ">>") == 0);
                                outfile = argv[k+1];
                                argv[k] = NULL;


                                break;
                            }
                        }

                        //construccion con sin simbolos
                        char *with_time[64];
                        int i = 0;
                        with_time[i] = "time";
                        i++;

                        for (int j = 3; argv[j] != NULL; j++) {
                            with_time[i] = argv[j];
                            i++;
                        }
                        with_time[i] = NULL;


                        int timemaximo = atoi(argv[2]); 
                        pid_t pid = fork();
                        if (pid == 0) {
                            
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

                            execvp(with_time[0], with_time);
                            perror("execvp");
                            exit(1);

                        } else if (pid > 0) {
                            // padre espera el tiempo
                            int waited = 0, status;
                            while (waited < timemaximo) {
                                if (waitpid(pid, &status, WNOHANG) != 0) break;
                                sleep(1);
                                waited++;
                            }

                            if (waited >= timemaximo) {
                                printf("Tiempo maximo alcanzado\n");
                                kill(pid, SIGKILL);
                            }
                            //si no se mata el proseso muestra info a pantalla
                            else{
                                print_mem_peak(stdout);
                            }
                        } else {
                            perror("fork");
                            exit(1);
                        }


                    }
                    else{
                        printf("Error: falta de parametros en miprof\n");
                        printf("Uso correcto: miprof [ejec/ejecsave archivo/ejecutar maxtiempo] comando args\n");
                        exit(1); 
                    }
                    
                }
                else{
                    //si no se lee myprof

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
