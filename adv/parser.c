#include "headers.h"
/*
5 : io_op
1 : cmd
2 : args
3 : lim
4 : special
 */

char spec_chs[] = {
	'!',
	'(',
	')',
	';',
	'&',
	'|',
	'<',
	'>',
	' ',
	'\\',
	'\0'};

int point_len(char *arr)
{
	int tmp = 0;
	while (arr[tmp] != '\0')
		tmp++;

	tmp--;
	return tmp;
}

int check_char_in_list(char c, char *list, int size)
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

char **parse_line(char *buffer, int **tokens_type)
{
	char **tokens_tok = calloc(128, sizeof(char *));
	*tokens_type = calloc(128, sizeof(int));
	int tk_ind = 0;
	int bf_str = 0;
	int bf_end = 0;

	// Parsing.
	// It's ugly, but it allow me to change and add operators easily. Will refactor at the end.
	while (buffer[bf_end] != '\0')
	{
		if (buffer[bf_end] == '-' && buffer[bf_end - 1] == '&')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 2;
			tk_ind++;
		}
		else if (buffer[bf_end] == '(' || buffer[bf_end] == ')')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 4;
			tk_ind++;
		}
		else if (buffer[bf_end] == '!')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 4;
			tk_ind++;
		}
		else if (buffer[bf_end] == ';')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 3;
			tk_ind++;
		}
		else if (buffer[bf_end] == '&' && buffer[bf_end + 1] == '&')
		{
			bf_end++;
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 3;
			tk_ind++;
		}
		else if (buffer[bf_end] == '|')
		{
			if (buffer[bf_end + 1] == '|')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 3;
			tk_ind++;
		}
		else if (buffer[bf_end] == '<')
		{
			if (buffer[bf_end + 1] == '<' || buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 5;
			tk_ind++;
		}
		else if (buffer[bf_end] == '>')
		{
			if (buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 5;
			tk_ind++;
		}
		else if (isdigit(buffer[bf_end]))
		{
			while (isdigit(buffer[bf_end + 1]))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			//*tokens_type[tk_ind] = 2;
			tk_ind++;
		}
		if (buffer[bf_end] == ' ')
		{
		}
		else
		{
			while (!check_char_in_list(buffer[bf_end + 1], spec_chs, sizeof(spec_chs)))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind]);
			*tokens_type[tk_ind] = 1;
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