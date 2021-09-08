#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAX_STRING_LENGTH 1000

int LAST_RETURN_CODE = 0;

typedef int (*CommandPtr)(int, char**);

int echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
    }
    return argc - 1;
}

int retcode(int argc, char *argv[]) {
    printf("%d\n", LAST_RETURN_CODE);
    return 0;
}

enum ExecutionStatus {
    SUCCESS,
    COMMAND_NOT_FOUND
};

typedef struct {
    char **atoms;
    size_t length;
} ParsedInput;

typedef struct {
    enum ExecutionStatus status;
    int returnCode;
} ExecutedInput;

ParsedInput parseInput(char *str) {
    size_t spaceCount = 0;
    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == ' ') spaceCount++;
    }
    size_t length = spaceCount + 1;
    char** atoms = malloc(sizeof(char*) * length);
    char* atom = strtok(str, " ");
    for(int i = 0; atom != NULL; i++) {
        atoms[i] = malloc(sizeof(char) * strlen(atom));
        strcpy(atoms[i], atom);
        atom = strtok(NULL, " ");
    }
    return (ParsedInput){.atoms = atoms, .length = length};
}

CommandPtr getPointOfEntry(char* str) {
    if(strcmp(str, "echo") == 0) return echo;
    else if(strcmp(str, "retcode") == 0) return retcode;
    else return NULL;
}

ExecutedInput executeInput(ParsedInput input) {
    CommandPtr command = getPointOfEntry(input.atoms[0]);
    if(command == NULL) return (ExecutedInput){.status = COMMAND_NOT_FOUND, .returnCode = -1};
    else return (ExecutedInput){.status = SUCCESS, .returnCode = command(input.length, input.atoms)};
}



int main(int argc, char *argv[]) {
    char str[MAX_STRING_LENGTH + 1];
    while (fgets(str, MAX_STRING_LENGTH, stdin)) {
        char* input = strtok(str, ";");
        while (input != NULL) {
            ParsedInput parsedInput = parseInput(input);
            ExecutedInput executedInput = executeInput(parsedInput);
            switch (executedInput.status) {
                case SUCCESS:
                    LAST_RETURN_CODE = executedInput.returnCode;
                    break;
                case COMMAND_NOT_FOUND:
                    fprintf(stderr, "Command not found");
                    break;
            }
            input = strtok(NULL, ";");
        }
    }
    return 0;
}
