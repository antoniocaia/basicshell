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


char spec_chs[] = {
	'!', '(', ')', ';', '&', '|', '<', '>', ' ', '\\', '\0'};


int read_input(char **buffer);
char **parse_line(char *buffer);
int tokenizer(char *buffer, char **args, char *separator);
void insert_token(char *buffer, int bf_str, int bf_end, char **line_tokens);
void execute(char **);

int point_len(char *arr);
int is_char_in_list(char c, char *list, int size);

//-------------------------------------------

int point_len(char *arr)
{
	int tmp = 0;
	while (arr[tmp] != '\0')
		tmp++;

	tmp--;
	return tmp;
}

int is_char_in_list(char c, char *list, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (list[i] == c)
			return 1;
	}
	return 0;
}

void insert_token(char *buffer, int bf_str, int bf_end, char **line_tokens)
{
	int str_len = &buffer[bf_end] - &buffer[bf_str] + 1;
	*line_tokens = calloc(str_len, sizeof(char));
	strncpy(*line_tokens, &buffer[bf_str], str_len);
}

char **parse_line(char *buffer)
{
	char **tokens_tok = calloc(128, sizeof(char *));
	char **tokens_type = calloc(128, sizeof(char *));
	int tk_ind = 0;
	int bf_str = 0;
	int bf_end = 0;

	// Parsing.
	// It's ugly, but it allow me to change and add operators easily. Will refactor at the end.
	while (buffer[bf_end] != '\0')
	{
		if (buffer[bf_end] == ' ')
		{
		}
		else if (buffer[bf_end] == ';' || buffer[bf_end] == '(' || buffer[bf_end] == ')' || buffer[bf_end] == '!' || (buffer[bf_end] == '-' && buffer[bf_end - 1] == '&'))
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}
		else if (buffer[bf_end] == '&')
		{
			if (buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tokens_type = 1;
			tk_ind++;
		}
		else if (buffer[bf_end] == '|')
		{
			if (buffer[bf_end + 1] == '|')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}
		else if (buffer[bf_end] == '<')
		{
			if (buffer[bf_end + 1] == '<' || buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}
		else if (buffer[bf_end] == '>')
		{
			if (buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}
		else if (isdigit(buffer[bf_end]))
		{
			while (isdigit(buffer[bf_end + 1]))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}
		else
		{
			while (!is_char_in_list(buffer[bf_end + 1], spec_chs, sizeof(spec_chs)))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			tk_ind++;
		}

		bf_end++;
		bf_str = bf_end;
	}
	return tokens_tok;
}

int tokenizer(char *buffer, char **args, char *separator)
{
	int index = 0;
	args[index] = strtok(buffer, separator);
	if (args[index] == NULL)
	{
		printf("Error parsing with  '%s'", separator);
		return -1;
	}

	do
	{
		index++;
		args[index] = strtok(NULL, separator);
	} while (args[index] != NULL);
	return 0;
}

int read_input(char **buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(buffer, &size, stdin);

	if (bytes_read == -1)
		exit(EXIT_SUCCESS);
	else if (bytes_read == 1)
		return bytes_read;

	if ((*buffer)[bytes_read - 1] == '\n')
		(*buffer)[bytes_read - 1] = '\0';
	return bytes_read;
}

int main(int argc, char **argv)
{
	while (true)
	{
		printf("> ");
		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == 1)
			continue;

		char **line_tokens = parse_line(buffer);

		int i = 0;
		while (line_tokens[i] != 0)
		{
			printf("[%s] ", line_tokens[i]);
			i++;
		}
		printf("\n");
		execute(line_tokens);
	}

	exit(EXIT_SUCCESS);
}