/******includes******/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

/******structs********/

// Token for pars/exec
struct token
{
	char *value;
	int type;
} typedef tok;

/******headers*******/
int read_input(char **buffer);

int b_cd(char **args);
int b_help(char **args);
int b_ex(char **args);
int b_exec(char **args);

tok **parse_line(char *buffer);
int is_char_in_list(char c, char *list, int size);
void insert_token(char *buffer, int bf_str, int bf_end, tok **line_tokens, int type);

void execute(tok **tokens);
int check_string_in_list(char *s, char **list, int size);
bool isnumber(char *s);
void setup_io_red(int tks_ind, tok **tokens, int io_ind);
void setup_io_cpy(int tks_ind, tok **tokens, int io_ind);
void prep_command(tok **tokens, int tks_ind);
int standard_execute(char **args);

/*/******variables*****/
// Builtin
static char *lookup_funct[] = {
	"cd",
	"help",
	"exit",
	"exec"};

// Builtin
static int (*builtin_funct[])(char **) = {
	&b_cd,
	&b_help,
	&b_ex,
	&b_exec};

// Builtin
static int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

// Terminal
static bool has_exec_failed = false;
