#include "headers.h"

int read_input(char** buffer) {
	size_t buffer_size = 0;
	return getline(buffer, &buffer_size, stdin);
}

int main(int argc, char** argv) {
	// Init global vars
	g_token_number = 0;

	while (true) {
		// Terminal output
		printf("> ");
		// Read input
		char* buffer;
		size_t char_read = read_input(&buffer);
		if (char_read == -1) {
			// Deal with error 
			printf("getline failed: %s\n", strerror(errno));
		}
		else if (char_read == 1) {
			// no input
			continue;
		}
		// Lex the line in token 
		tok** token_list = lex_line(buffer);
		// Parse the token to index
		pn* root = parse(token_list);
		// Execut command

		// TEST OUTPUT TOKEN
		int i = 0;
		while (token_list[i] != 0) {
			printf("[%s]", token_list[i]->value);
			i++;
		}
		// END TEST OUTPUT
		printf("\n");
	}
	exit(EXIT_SUCCESS);
}