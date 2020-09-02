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



/******headers*******/
int read_input(char **buffer);

char **parse_line(char *buffer, int **tokens_type);
int point_len(char *arr);
int check_char_in_list(char c, char *list, int size);
int tokenizer(char *buffer, char **args, char *separator);
void insert_token(char *buffer, int bf_str, int bf_end, char **line_tokens);

void execute(char **tokens, int *types);
int check_string_in_list(char *s, char **list, int size);
int check_io_redirect(char **tokens);
bool isnumber(char *s);
void setup_io(int tks_ind, char **tokens, int io_ind);

