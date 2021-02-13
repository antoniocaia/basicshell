#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>		// errno
#include <string.h>		// strlng, ...
#include <unistd.h>		// chdir, execvp, ...
#include <sys/types.h>	// waitpid
#include <sys/wait.h>	// waitpid
#include <ctype.h>		// isdigit, isalpha, isspace


// LEX: token struct and type value, used by the lexer to easily build the tree
enum token_type { t_str, t_separator, t_number };

struct token {
	char* value;
	enum token_type type;
} typedef tok;

// PARS: node struct used by the parser to build a binary tree to define the execution flow
enum node_type { p_arg, p_separator, p_null };

struct parser_node {
	enum node_type type;
	char** args;
	struct parser_node* left;
	struct parser_node* rigth;
} typedef pn;

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
void insert_token(char* buffer, int bf_str, int bf_end, tok** line_tokens, int type);
char* read_another_line(char* buffer);
tok** lex_line(char* buffer);

// parser.c
char** toks_to_strings(tok** tokens);
pn* parse(tok** tokens);
pn* parsing(tok** tokens, int t_start, int t_end);

// execute.c
int is_builtin(char* cmd);
int execute_cmd(char** args);
int execute(pn* root);

// GLOBAL VAR
// Mostly size of struct
int g_token_number;