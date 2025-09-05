#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 100
#define true 1
#define false 0
#define bool char

// Buscar Ubuntu en tecla Windows
// cd /mnt/c/Users/kh/Desktop/shell

bool readLine(char* line) {
    printf("Ingrese un comando:\n");
    
    if (fgets(line, MAX_LINE_LENGTH, stdin) != NULL) {
        return true;
    } else {
        printf("Error al leer la entrada.");
    }
    return false;
}

int main() {
    char line[MAX_LINE_LENGTH];
    pid_t pid;

    while (readLine(line)) {
        pid = fork();
        if (pid == 0) {
            printf("Proceso hijo ejecutando [%d]:\n", getpid());
            printf("%s", line);
            return 0;
            //exec(line);
        } else {
            printf("Proceso padre ejecutando [%d]:\n", getpid());
            wait(NULL);
        }
    }

    return 0;
}