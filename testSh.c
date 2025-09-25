#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char input[1024];

    while (1) {
        printf("comando: ");
        if (!fgets(input, sizeof(input), stdin)) break;

        if (strncmp(input, "exit", 4) == 0) break;

        input[strcspn(input, "\n")] = 0;

        pid_t pid = fork();
        if (pid == 0) {
            execlp("sh", "sh", "-c", input, NULL);
            perror("execlp");
            exit(1);
        }

        wait(NULL);
    }

    return 0;
}