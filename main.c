#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

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
    COMMAND_NOT_FOUND,
    EMPTY_INPUT
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
    size_t wordsCount = 0;
    int state = 0;
    for(int i = 0; i < strlen(str); i++) {
        if(state == 0) {
            if(isblank(str[i]) == 0) {
                wordsCount++;
                state = 1;
            }
        }
        else {
            if(isblank(str[i]) != 0) state = 0;
        }
    }
    if(wordsCount == 0) return (ParsedInput){.atoms = NULL, .length = 0};

    char** words = malloc(sizeof(char*) * wordsCount);
    char* savePtr;
    char* word = __strtok_r(str, " ", &savePtr);
    for(int i = 0; word != NULL && i < wordsCount; i++) {
        words[i] = malloc(sizeof(char) * (strlen(word) + 1));
        strcpy(words[i], word);
        word = __strtok_r(NULL, " ", &savePtr);
    }
    return (ParsedInput){.atoms = words, .length = wordsCount};
}

void freeParsedInput(ParsedInput* parsedInput) {
    if(parsedInput == NULL) return;
    if(parsedInput->atoms == NULL) return;

    for(int i = 0; i < parsedInput->length; i++) free(parsedInput->atoms[i]);
    free(parsedInput->atoms);
}

CommandPtr getPointOfEntry(char* str) {
    if(strcmp(str, "echo") == 0) return echo;
    else if(strcmp(str, "retcode") == 0) return retcode;
    else return NULL;
}

ExecutedInput executeParsedInput(ParsedInput input) {
    if(input.length == 0 || input.atoms == NULL) return (ExecutedInput){.status = EMPTY_INPUT, .returnCode = -1};

    CommandPtr command = getPointOfEntry(input.atoms[0]);
    if(command == NULL) return (ExecutedInput){.status = COMMAND_NOT_FOUND, .returnCode = -1};
    else return (ExecutedInput){.status = SUCCESS, .returnCode = command(input.length, input.atoms)};
}

void evaluate(char* input) {
    ParsedInput parsedInput = parseInput(input);
    ExecutedInput executedInput = executeParsedInput(parsedInput);
    switch (executedInput.status) {
        case SUCCESS:
            LAST_RETURN_CODE = executedInput.returnCode;
            break;
        case COMMAND_NOT_FOUND:
            fprintf(stderr, "Command not found\n");
            break;
        case EMPTY_INPUT:
            fprintf(stderr, "Empty input\n");
            break;
    }
    freeParsedInput(&parsedInput);
}

int main(int argc, char *argv[]) {
    char str[MAX_STRING_LENGTH + 1];
    while (fgets(str, MAX_STRING_LENGTH, stdin)) {
        char* savePtr;
        char* input = __strtok_r(str, "\n;", &savePtr);
        while (input != NULL) {
            evaluate(input);
            input = __strtok_r(NULL, "\n;", &savePtr);
        }
    }
    return 0;
}
