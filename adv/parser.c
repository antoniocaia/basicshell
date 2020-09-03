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

int is_char_in_list(char c, char *list, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (list[i] == c)
			return 1;
	}
	return 0;
}

void insert_token(char *buffer, int bf_str, int bf_end, tok **line_tokens, int type)
{
	int str_len = &buffer[bf_end] - &buffer[bf_str] + 1;
	*line_tokens = malloc(sizeof(tok));
	(*line_tokens)->value = calloc(str_len, sizeof(char));
	strncpy((*line_tokens)->value, &buffer[bf_str], str_len);
	(*line_tokens)->type = type;
}

int point_len(char *arr)
{
	int tmp = 0;
	while (arr[tmp] != '\0')
	{
		tmp++;
	}
	tmp--;
	return tmp;
}

tok **parse_line(char *buffer)
{
	tok **tokens_tok = calloc(128, sizeof(tok *));
	int tk_ind = 0;
	int bf_str = 0;
	int bf_end = 0;

	if (buffer[point_len(buffer)] == '\\')
	{
		printf("> ");
		char *new_line;
		read_input(&new_line);
		char *new_buffer;
		new_buffer = calloc(64, sizeof(char));
		buffer[strlen(buffer) - 1] = '\0';
		strcpy(new_buffer, buffer);
		strcat(new_buffer, new_line);
		return parse_line(new_buffer);
	}

	// Parsing.
	// It's ugly, but it allow me to change and add operators easily. Will refactor at the end.
	while (buffer[bf_end] != '\0')
	{
		if (buffer[bf_end] == '-' && buffer[bf_end - 1] == '&')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 2);
			tk_ind++;
		}
		else if (buffer[bf_end] == '(' || buffer[bf_end] == ')')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 4);
			tk_ind++;
		}
		else if (buffer[bf_end] == '!')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 4);
			tk_ind++;
		}
		else if (buffer[bf_end] == ';')
		{
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 3);
			tk_ind++;
		}
		else if (buffer[bf_end] == '&' && buffer[bf_end + 1] == '&')
		{
			bf_end++;
			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 3);
			tk_ind++;
		}
		else if (buffer[bf_end] == '|')
		{
			if (buffer[bf_end + 1] == '|')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 3);
			tk_ind++;
		}
		else if (buffer[bf_end] == '<')
		{
			if (buffer[bf_end + 1] == '<' || buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 5);
			tk_ind++;
		}
		else if (buffer[bf_end] == '>')
		{
			if (buffer[bf_end + 1] == '>' || buffer[bf_end + 1] == '&')
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 5);
			tk_ind++;
		}
		else if (isdigit(buffer[bf_end]))
		{
			while (isdigit(buffer[bf_end + 1]))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 2);
			tk_ind++;
		}
		else if (buffer[bf_end] == ' ')
		{
		}
		else
		{
			while (!is_char_in_list(buffer[bf_end + 1], spec_chs, sizeof(spec_chs)))
				bf_end++;

			if (tk_ind > 0 && (tokens_tok[tk_ind - 1]->type == 1 || tokens_tok[tk_ind - 1]->type == 5 || tokens_tok[tk_ind - 1]->type == 2))
				insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 2);
			else
				insert_token(buffer, bf_str, bf_end, &tokens_tok[tk_ind], 1);
			tk_ind++;
		}

		bf_end++;
		bf_str = bf_end;
	}
	return tokens_tok;
}