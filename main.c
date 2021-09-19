#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "pool.h"

#define MAX_STRING_LENGTH 1000

int LAST_RETURN_CODE = 0;

typedef int (*CommandPtr)(int, char**);

#define APPS_X(X) \
        X(echo) \
        X(retcode) \
        X(pooltest)\

#define DECLARE(X) static int X(int, char *[]);
APPS_X(DECLARE)
#undef DECLARE

static const struct command {
    const char *name;
    CommandPtr cmd;
} command_list[] = {
#define ELEM(X) { # X, X },
        APPS_X(ELEM)
#undef ELEM
};

static int echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
    }
    return argc - 1;
}

static int retcode(int argc, char *argv[]) {
    printf("%d\n", LAST_RETURN_CODE);
    return 0;
}

static int pooltest(int argc, char *argv[]) {
    struct obj {
        void *field1;
        void *field2;
    };
    static struct obj objmem[4];
    static struct pool objpool = POOL_INITIALIZER_ARRAY(objmem);

    if (!strcmp(argv[1], "alloc")) {
        struct obj *o = pool_alloc(&objpool);
        printf("alloc %d\n", o ? (o - objmem) : -1);
        return 0;
    } else if (!strcmp(argv[1], "free")) {
        int iobj = atoi(argv[2]);
        printf("free %d\n", iobj);
        pool_free(&objpool, objmem + iobj);
        return 0;
    }
}

enum ExecutionStatus {
    SUCCESS,
    COMMAND_NOT_FOUND,
    EMPTY_INPUT
};

typedef struct {
    char **argv;
    size_t argc;
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
    if(wordsCount == 0) return (ParsedInput){.argv = NULL, .argc = 0};

    char** words = malloc(sizeof(char*) * wordsCount);
    char* savePtr;
    char* word = __strtok_r(str, " ", &savePtr);
    for(int i = 0; word != NULL && i < wordsCount; i++) {
        words[i] = malloc(sizeof(char) * (strlen(word) + 1));
        strcpy(words[i], word);
        word = __strtok_r(NULL, " ", &savePtr);
    }
    return (ParsedInput){.argv = words, .argc = wordsCount};
}

void freeParsedInput(ParsedInput* parsedInput) {
    if(parsedInput == NULL) return;
    if(parsedInput->argv == NULL) return;

    for(int i = 0; i < parsedInput->argc; i++) free(parsedInput->argv[i]);
    free(parsedInput->argv);
}

CommandPtr getPointOfEntry(char* str) {
    for(int i = 0; i < sizeof(command_list) / sizeof(command_list[0]); i++) {
        if(strcmp(str, command_list[i].name) == 0) return command_list[i].cmd;
    }
    return NULL;
}

ExecutedInput executeParsedInput(ParsedInput input) {
    if(input.argc == 0 || input.argv == NULL) return (ExecutedInput){.status = EMPTY_INPUT, .returnCode = -1};

    CommandPtr command = getPointOfEntry(input.argv[0]);
    if(command == NULL) return (ExecutedInput){.status = COMMAND_NOT_FOUND, .returnCode = -1};
    else return (ExecutedInput){.status = SUCCESS, .returnCode = command(input.argc, input.argv)};
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
