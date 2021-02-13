#include "headers.h"

// DEBUG
// TEST TOKEN
void print_tokens(tok** tl) {
	int i = 0;
	while (tl[i] != 0) {
		printf("[%s]", tl[i]->value);
		i++;
	}
	printf("\n\n");
}
// DEBUG
// TEST NODE
void print_pars(pn* root) {
	if(root == NULL) return;
	printf("Type [%d]\n", root->type);
	int i = 0;
	while ((root->args)[i] != 0) {
		printf("[%s]", (root->args)[i]);
		i++;
	}
	printf("\n\n");
	print_pars(root->left);
	print_pars(root->rigth);
}

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
			printf("getline failed: %s\n", strerror(errno));
		}
		else if (char_read == 1) {
			// no input
			continue;
		}
		g_token_number = 0;
		// Lex the line in token 
		tok** token_list = lex_line(buffer);
		
		// DEBUG
		print_tokens(token_list);
		
		// Parse the token to index
		pn* root = parse(token_list);
		
		// DEBUGG
		print_pars(root);
		
		// Execut command
		execute(root);

		printf("\n");
	}
	exit(EXIT_SUCCESS);
}