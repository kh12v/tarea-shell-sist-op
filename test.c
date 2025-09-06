#include <string.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 100
#define true 1
#define false 0
#define bool char

bool readLine(char* line) {
    printf("Ingrese un comando:\n");
    
    if (fgets(line, MAX_LINE_LENGTH, stdin) != NULL) {
        return true;
    } else {
        printf("Error al leer la entrada.");
    }
    return false;
}

void readCommand(char* line, char** program, char* args[]) {
    char* word = strtok (line, " ");
    *program = word;
    printf("1 program: %s\n", *program);
    int n = 0;
    while (word != NULL) {
        printf ("%s/", word);
        word = strtok (NULL, " ");
        args[n] = word;
        n++;
    }
    args[n] = NULL;
    printf("2 program: %s\n", *program);
}

int main() {
    char line[MAX_LINE_LENGTH];
    char* program;
    char* args[MAX_LINE_LENGTH];

    readLine(line);
    readCommand(line, &program, args);

    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        if (args[i] == NULL) {
            break;
        }
    }

    return 0;
}