#include "headers.h"

int read_input(char** buffer) {
	size_t buffer_size = 0;
	return getline(buffer, &buffer_size, stdin);
}

int main(int argc, char** argv) {

	while (true) {
		// Terminal output
		printf("> ");
		// Read input
		char* buffer;
		size_t char_read = read_input(&buffer);
		if (char_read == -1) {
			// Deal with error 
			printf("getline interrupted: %s\n", strerror(errno));
			exit(EXIT_SUCCESS);
		}
		else if (char_read == 1) {
			// no input
			continue;
		}
		g_token_number = 0;
		// Lex the line in token 
		tok** token_list = lex_line(buffer);
		print_tokens(token_list); 				// DEBUG
		// Parse the token to index
		pn* root = parse(token_list);
		print_pars(root); 						// DEBUG
		// Execut command
		execute(root);

		printf("\n");
	}
	exit(EXIT_SUCCESS);
}