#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>		// errno
#include <string.h>		// strlng, ...
#include <unistd.h>		// chdir, execvp, ...
#include <sys/types.h>	// waitpid
#include <sys/wait.h>	// waitpid

struct token {
	char* value;
} typedef tok;

// main.c
int read_input(char** buffer);

// builtin.c
int b_cd(char** args);
int b_help(char** args);
int b_ex(char** args);
int b_exec(char** args);

// Builtin look up table to match command name with function
static char* lookup_funct[] = {
	"cd",
	"help",
	"exit",
	"exec"
};

// Builtin function references
static int (*builtin_funct[])(char**) = {
	&b_cd,
	&b_help,
	&b_ex,
	&b_exec
};

static int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

// lexer.c
void insert_token(char* str, tok** token_list);
tok** lex_line(char* buffer);

// parser.c
char** toks_to_strings(tok** tokens);

// execute.c
int is_builtin(char* cmd);
int execute(char** args);


