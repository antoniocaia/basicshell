#include "headers.h"

pn* parse(tok** tokens) {
	pn* root = malloc(sizeof(pn));
	parsing(root, tokens, 0, g_token_number - 1);
	return root; // Can't return local var address!
}

void parsing(pn* node, tok** tokens, int t_start, int t_end) {
	node = malloc(sizeof(pn));
	node->left = NULL;
	node->rigth = NULL;
	node->value = NULL;

	// Check for every token that separate multiple commands, and end the function call
	int t_current = t_start;
	while (t_current < t_end) {
		if (tokens[t_current]->type == t_separator) {
			node->type = p_separator;
			node->value = ";";

			parsing(node->left, tokens, t_start, t_current - 1);
			parsing(node->rigth, tokens, t_current + 1, t_end);
			return;
		}
		t_current++;
	}

	// No "special token" means that what remains is commands and args
	// Set the value filed and the args vector
	node->value = tokens[t_start]->value;
	node->args = calloc(16, sizeof(char*));
	(node->args)[0] = tokens[t_start]->value;

	int args_ind = 1;
	t_current = t_start + 1;
	while (t_current < t_end) {
		(node->args)[args_ind] = tokens[t_current]->value;
		t_current++;
	}
}

char** toks_to_strings(tok** tokens) {
	int tks_ind = 0;
	int args_ind = 0;
	// A command can support max 15 args
	char** args = calloc(16, sizeof(char*));
	args[args_ind] = tokens[tks_ind]->value;
	args_ind++;
	tks_ind++;

	while (tokens[tks_ind] != 0) {
		args[args_ind] = tokens[tks_ind]->value;
		args_ind++;
		tks_ind++;
	}
	return args;
}
