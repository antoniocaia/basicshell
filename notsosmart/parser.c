#include "headers.h"

pn* parse(tok** tokens) {
	pn* root = parsing(tokens, 0, g_token_number - 1);
	return root;
}

pn* parsing(tok** tokens, int tok_start, int tok_end) {
	// No more tokens
	if (tok_start > tok_end) return NULL;

	pn* node = malloc(sizeof(pn));
	node->type = p_null;
	node->left = NULL;
	node->rigth = NULL;

	// Subshell (round brackets)
	if (tokens[tok_start]->type == t_leftb && tokens[tok_end]->type == t_rigthb) {
		// If all the current segment is between two brackets we can process it a normal sequence
		node->type = p_subshell;
		node->args = calloc(1, sizeof(char*));
		(node->args)[0] = "()";
		node->left = parsing(tokens, tok_start + 1, tok_end - 1);
		return node;
	}

	// Check for every token that separate multiple commands, and end the function call
	int tok_current = tok_start;
	while (tok_current <= tok_end) {
		// When finding round brackets, we skip every token in the middle
		if (tokens[tok_current]->type == t_leftb) {
			while (tokens[tok_current]->type != t_rigthb)
				tok_current++;
		}
		else if (tokens[tok_current]->type == t_separator) {
			node->type = p_separator;
			node->args = calloc(1, sizeof(char*));
			(node->args)[0] = ";";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		else if (tokens[tok_current]->type == t_and) {
			node->type = p_and;
			node->args = calloc(1, sizeof(char*));
			(node->args)[0] = "&&";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		else if (tokens[tok_current]->type == t_or) {
			node->type = p_or;
			node->args = calloc(1, sizeof(char*));
			(node->args)[0] = "||";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		tok_current++;
	}

	// Check for bang '!' operator
	if (tokens[tok_start]->type == t_bang) {
		node->type = p_bang;
		node->args = calloc(1, sizeof(char*));
		(node->args)[0] = "!";
		node->left = parsing(tokens, tok_start + 1, tok_end);
		return node;
	}

	// No "special token" means that what remains is commands and args
	// Set the args vector
	node->type = p_arg;
	node->args = calloc(16, sizeof(char*));
	(node->args)[0] = tokens[tok_start]->value;

	int args_ind = 1;
	tok_current = tok_start + 1;
	while (tok_current <= tok_end) {
		(node->args)[args_ind] = tokens[tok_current]->value;
		tok_current++;
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
