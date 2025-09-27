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

int main() {
    char input[1024];
    char inputCopy[1024];

    while (1) {
        printf("Comando: ");
        if (!fgets(input, sizeof(input), stdin)) break;

        if (strncmp(input, "exit", 4) == 0) exit(0);

        input[strcspn(input, "\n")] = 0;


        pid_t pid = fork();
        if (pid == 0) {
            strcpy(inputCopy, input);
            char** argv = parse_command(inputCopy);

            if(argv[0] && strcmp(argv[0], "miprof") == 0){
                //seccion con myprof
                if(argv[1] && strcmp(argv[1], "ejec") == 0){
                    //muestra a pantalla

                    //construccion con sin simbolos
                    char command[1024] = "time";
                    for (int j = 2; argv[j] != NULL; j++) {
                        strcat(command, " ");
                        strcat(command, argv[j]);
                    }

                    pid_t cpid = fork();
                    if (cpid == 0) {
                        execlp("sh", "sh", "-c", command, (char *)NULL);
                        perror("execlp");
                        exit(1);
                    } else {
                        waitpid(cpid, NULL, 0);
                        print_mem_peak(stdout);
                        exit(0);
                    }
                }
                else if (argv[1] && strcmp(argv[1], "ejecsave") == 0){
                    //guarda en un archivo

                    FILE *archivo; 
                    char *nombre = argv[2];
                    archivo = fopen(nombre, "a");

                    //construccion con sin simbolos
                    char command[1024] = "time";
                    for (int j = 2; argv[j] != NULL; j++) {
                        strcat(command, " ");
                        strcat(command, argv[j]);
                    }

                    //lo mismo que el anterior
                    pid_t cpid = fork();
                    if (cpid == 0) {
                        //almacena time
                        int fd_archivo = fileno(archivo);
                        if (dup2(fd_archivo, STDERR_FILENO) == -1) {
                            perror("dup2 stderr");
                            exit(1);
                        }
                        
                        execlp("sh", "sh", "-c", command, (char *)NULL);
                        perror("execlp");
                        exit(1);
                    } else {
                        waitpid(cpid, NULL, 0);
                        // Peak al archivo
                        print_mem_peak(archivo);
                        fclose(archivo);
                        exit(0);
                    }

                }
                else if (argv[1] && strcmp(argv[1], "ejecutar") == 0){
                    //ejecucion con tiempo limite

                    //construccion con sin simbolos
                    char command[1024] = "time";
                    for (int j = 3; argv[j] != NULL; j++) {
                        strcat(command, " ");
                        strcat(command, argv[j]);
                    }

                    int timemaximo = atoi(argv[2]); 
                    pid_t pid = fork();
                    if (pid == 0) {
                        execlp("sh", "sh", "-c", command, (char *)NULL);
                        perror("execlp");
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
                            exit(0);
                        }
                        //si no se mata el proseso muestra info a pantalla
                        else{
                            print_mem_peak(stdout);
                            exit(0);
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

                execlp("sh", "sh", "-c", input, NULL);
                perror("execvp");
                exit(1);
            }
        }
        wait(NULL);
    }

    return 0;
}
