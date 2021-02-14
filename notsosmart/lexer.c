#include "headers.h"

void insert_token(char* buffer, int bf_str, int bf_end, tok** line_tokens, int type) {
	int str_len = bf_end - bf_str + 1;
	// printf("STRING LEN %d \n start %d \n end %d\n\n", str_len, bf_str, bf_end); //DEBUGG
	*line_tokens = malloc(sizeof(tok));
	(*line_tokens)->value = calloc(str_len, sizeof(char));
	strncpy((*line_tokens)->value, &buffer[bf_str], str_len);
	(*line_tokens)->type = type;

	g_token_number++;
}

char* read_another_line(char* buffer) {
	printf("\\> ");

	int buff_len = strlen(buffer);
	buffer[buff_len - 1] = '\0';

	char* new_line;
	read_input(&new_line);

	char* new_buffer;
	new_buffer = calloc(64, sizeof(char));
	strcpy(new_buffer, buffer);
	strcat(new_buffer, new_line);
	return new_buffer;
}

bool is_string(char* buffer, int bf_end) {
	return isalpha(buffer[bf_end])
		|| isdigit(buffer[bf_end])
		|| buffer[bf_end] == '_'
		|| buffer[bf_end] == '-'
		|| buffer[bf_end] == '.'
		|| buffer[bf_end] == '/';
}

tok** lex_line(char* buffer) {
	int buffer_size = strlen(buffer) - 1;
	// Remove end-line char (\n)
	buffer[buffer_size] = '\0';
	buffer_size -= 1;
	// Check if '\' operator is at the end of the input; if true read another line from stdin
	if (buffer[buffer_size] == '\\') {
		// the new call of lex_buffer replace the current one
		return lex_line(read_another_line(buffer));
	}

	// Allocate space for max 128 tokens
	tok** tokens = calloc(128, sizeof(tok*));
	int tk_ind = 0;
	// Starting index for the current token
	int bf_str = 0;
	// Ending index for the current token
	int bf_end = 0;

	while (buffer[bf_end] != '\0') {
		if (isspace(buffer[bf_end])) {
			// Skip every space inside the string
		}
		else if (buffer[bf_end] == '!') {		// Output negation
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_bang);
			tk_ind++;
		}
		else if (buffer[bf_end] == '&' && buffer[bf_end + 1] == '&') {	// AND
			bf_end++;
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_and);
			tk_ind++;

		}
		else if (buffer[bf_end] == '|' && buffer[bf_end + 1] == '|') {	// OR
			bf_end++;
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_or);
			tk_ind++;

		}
		else if (buffer[bf_end] == '|' && buffer[bf_end + 1] != '|') {	// PIPE
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_pipe);
			tk_ind++;
		}
		else if (buffer[bf_end] == ';') {			// Cmd separator
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_separator);
			tk_ind++;
		}
		else if (buffer[bf_end] == '(') {			// brackets (subshell)
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_leftb);
			tk_ind++;
		}
		else if (buffer[bf_end] == ')') {			// brackets (subshell)
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_rigthb);
			tk_ind++;
		}
		else if (isdigit(buffer[bf_end])) {			// Number
			while (isdigit(buffer[bf_end + 1]))
				bf_end++;
			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_number);
			tk_ind++;
		}
		else if (is_string(buffer, bf_end)) {		// String
			while (is_string(buffer, bf_end + 1))
				bf_end++;

			insert_token(buffer, bf_str, bf_end, &tokens[tk_ind], t_str);
			tk_ind++;
		}
		else {
			char* invalid_str = malloc(sizeof(char) * 64);
			int str_len = bf_end - bf_str + 1;
			strncpy(invalid_str, &buffer[bf_str], str_len);
			printf("LEX: invalid sign: [%s]\n", invalid_str);
		}

		bf_end++;
		bf_str = bf_end;
	}
	return tokens;
}