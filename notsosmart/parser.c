#include "headers.h"

pn* parse(tok** tokens) {
	pn* root = parsing(tokens, 0, g_token_number - 1);
	return root;
}

pn* parsing(tok** tokens, int t_start, int t_end) {
	// No more tokens
	if (t_start > t_end) return NULL;

	pn* node = malloc(sizeof(pn));
	node->type = p_null;
	node->left = NULL;
	node->rigth = NULL;

	// Check for every token that separate multiple commands, and end the function call
	int t_current = t_start;
	while (t_current <= t_end) {
		if (tokens[t_current]->type == t_separator) {
			node->type = p_separator;
			node->args = calloc(1, sizeof(char*));
			(node->args)[0] = ";";

			node->left = parsing(tokens, t_start, t_current - 1);
			node->rigth = parsing(tokens, t_current + 1, t_end);
			return node;
		}
		t_current++;
	}

	// No "special token" means that what remains is commands and args
	// Set the args vector
	node->type = p_arg;
	node->args = calloc(16, sizeof(char*));
	(node->args)[0] = tokens[t_start]->value;

	int args_ind = 1;
	t_current = t_start + 1;
	while (t_current <= t_end) {
		(node->args)[args_ind] = tokens[t_current]->value;
		t_current++;
		args_ind++;
	}
	return node;
}

// DEBUGG
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
