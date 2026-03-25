/*
THE BOL PROGRAMMING LANGUAGE

this project is a interpreter for this language, not a compiler

variables can be named only as lowercase characters, example: a, i, j, x, y, z

objects:

char - single character, example: a, b, c, 1, 3, |, ., ^, $
string - a list of characters, that is declared using braces, example: "Hello, World!"
number - declared using parentheses, example: (67), (0x15)

operators:

O (variable or string or number) - print into standard output
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 32
#define FIRST_VAR_NAME 'a'
#define LAST_VAR_NAME 'z'
#define NUM_VARS LAST_VAR_NAME - FIRST_VAR_NAME + 1

#define isVariable(c) c >= FIRST_VAR_NAME && c <= LAST_VAR_NAME
#define isNumber(c) c >= '0' && c <= '9'
#define getValueVar(c) c >= FIRST_VAR_NAME && c <= LAST_VAR_NAME

typedef enum : char {
    SCOPE_NONE,
    SCOPE_ERROR,
    SCOPE_ASSIGNVAR,
    SCOPE_ASSIGNVAR_NUMBER,
    SCOPE_ASSIGNVAR_STRING,
    SCOPE_OUTPUT,
    SCOPE_OUTPUT_NUMBER,
    SCOPE_OUTPUT_STRING
} Scope;

typedef uint_fast8_t VarID;

static VarID currentVar;

static size_t bufferLength;
static char buffer[BUFFER_SIZE];

static size_t varSizes[NUM_VARS];
static void* vars[NUM_VARS];

static VarID getVar(const char c) {
    return c - FIRST_VAR_NAME;
}
static Scope processCharacter(const char c, const Scope scope) {
    switch (scope) {
    case SCOPE_NONE:
	if (isVariable(c)) {
	    currentVar = getVar(c);

	    return SCOPE_ASSIGNVAR;
	}

	switch (c) {
	case 'O':
	    return SCOPE_OUTPUT;
	case '\n':
	case '\r':
	    //carriage return and new line
	    return scope;
	case ' ':
	    puts("\nSpaces are forbidden in the bol programming language! (except inside strings)");
	default:
	    printf("\nUnknown expression: %u", c);
	}

	return SCOPE_ERROR;
    case SCOPE_OUTPUT:
	if (isVariable(c)) {
	    const void* const data = vars[getVar(c)];

	    if (data) {
		puts(data);

		return SCOPE_NONE;
	    }

	    printf("\nTried to print an uninitalized variable: %c\n", c);

	    return SCOPE_ERROR;
	}

	switch (c) {
	case '"':
	    return SCOPE_OUTPUT_STRING;
	case '(':
	    bufferLength = 1;

	    return SCOPE_OUTPUT_NUMBER;
	}

	printf("\nUnknown expression: O%c", c);

	return SCOPE_ERROR;
    case SCOPE_OUTPUT_NUMBER:
	if (c == ')') {
	    buffer[bufferLength - 1] = '\0';

	    uint64_t number = strtoull(buffer, NULL, 0);

	    printf("%llu", number);

	    return SCOPE_NONE;
	}

	buffer[bufferLength++ - 1] = c;

	break;
    case SCOPE_OUTPUT_STRING:
	if (c == '"') return SCOPE_NONE;

	putchar(c);

	break;
    case SCOPE_ASSIGNVAR:
	if (isVariable(c)) {
	    if (vars[getVar(c)]) {
		if (vars[currentVar]) free(vars[currentVar]);

		vars[currentVar] = malloc(varSizes[getVar(c)]);
		varSizes[currentVar] = varSizes[getVar(c)];

		memcpy(vars[currentVar], vars[getVar(c)], varSizes[getVar(c)]);

		return SCOPE_NONE;
	    }

	    printf(
		"\nTried to assign a variable '%c' with a value of an uninitalized variable '%c'\n", 
		currentVar + FIRST_VAR_NAME, c
	    );

	    return SCOPE_ERROR;
	}

	switch (c) {
	case '"':
	    bufferLength = 1;

	    return SCOPE_ASSIGNVAR_STRING;
	}

	printf("\nUnknown expression: %c%c", currentVar + FIRST_VAR_NAME, c);

	return SCOPE_ERROR;
    case SCOPE_ASSIGNVAR_STRING:
	if (c == '"') {
	    buffer[bufferLength - 1] = '\0';

	    if (vars[currentVar]) free(vars[currentVar]);

	    varSizes[currentVar] = bufferLength;
	    vars[currentVar] = malloc(bufferLength);

	    memcpy(vars[currentVar], buffer, bufferLength);

	    return SCOPE_NONE;
	}

	buffer[bufferLength++ - 1] = c;

	break;
    default:
	if (1) {}
    }

    return scope;
}

int main(const int argc, const char* const argv[]) {
    if (argc > 1) {
	FILE* const file = fopen(argv[1], "rb");

	int c;

	Scope scope;

	if (!file) {
	    printf("Invalid file: %s\n", argv[1]);

	    return EXIT_FAILURE;
	}

	scope = SCOPE_NONE;

	while ((c = getc(file)) != EOF) {
	    if ((scope = processCharacter((char)c, scope)) == SCOPE_ERROR) return EXIT_FAILURE;
	}
    }
    else {
	puts("No source bol file!");

	return EXIT_FAILURE;
    }
}
