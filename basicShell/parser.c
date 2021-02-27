#include "headers.h"

pn* parse(tok** tokens) {
	pn* root = parsing(tokens, 0, g_token_number - 1);
	return root;
}

// Set up the correct node's parameters based on the io operator
void setup_io_pars(pn* node, int type) {
	if (type == t_rdm) {
		node->type = p_rdm;
		(node->args)[0] = ">";
	}
	else if (type == t_ldm) {
		node->type = p_ldm;
		(node->args)[0] = "<";
	}
	else if (type == t_rrdm) {
		node->type = p_rrdm;
		(node->args)[0] = ">>";
	}
	else if (type == t_lrdm) {
		node->type = p_lrdm;
		(node->args)[0] = "<>";
	}
	else if (type == t_endrdm) {
		node->type = p_endrdm;
		(node->args)[0] = "&>";
	}
	else if (type == t_ldmend) {
		node->type = p_ldmend;
		(node->args)[0] = "<&";
	}

}


/* 
The parser is probably the most delicate part of the shell.
the order in which the instructions are executed is not random: 
- check for round-brackets (subshell) so at execution time we create the subshell before executing every command between brackets
- check for "binary operator" that separate and connect different commands
At this point we have segments of the original input containing: 1. '!' 2. IO redirectors as '<', etc 3. Pipes 4. "standard" command
(Can't put pipe with the other "binary operator" (the logic would be the same) because how it interact with '!': 
'!' considere a pipeline as a unic command with one unique output)
- check for '!'
- check for pipes

*/

pn* parsing(tok** tokens, int tok_start, int tok_end) {
	// No more tokens
	if (tok_start > tok_end) return NULL;

	// Basic init
	pn* node = malloc(sizeof(pn));
	node->type = p_null;
	node->left = NULL;
	node->rigth = NULL;

	// Subshell (round brackets)
	if (tokens[tok_start]->type == t_leftbrack && tokens[tok_end]->type == t_rigthbrack) {
		// If all the current segment is between two brackets we can process it a normal sequence
		node->type = p_subshell;
		node->args = calloc(2, sizeof(char*));
		(node->args)[0] = "()";
		node->left = parsing(tokens, tok_start + 1, tok_end - 1);
		return node;
	}
	// Check for every binary operator that separate multiple commands
	int tok_current = tok_start;
	while (tok_current <= tok_end) {
		// When finding round brackets, we skip every token in the middle
		if (tokens[tok_current]->type == t_leftbrack) {
			while (tokens[tok_current]->type != t_rigthbrack)
				tok_current++;
		}

		else if (tokens[tok_current]->type == t_separator) {
			node->type = p_separator;
			node->args = calloc(2, sizeof(char*));
			(node->args)[0] = ";";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		else if (tokens[tok_current]->type == t_and) {
			node->type = p_and;
			node->args = calloc(2, sizeof(char*));
			(node->args)[0] = "&&";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		else if (tokens[tok_current]->type == t_or) {
			node->type = p_or;
			node->args = calloc(2, sizeof(char*));
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
		node->args = calloc(2, sizeof(char*));
		(node->args)[0] = "!";
		node->rigth = parsing(tokens, tok_start + 1, tok_end);
		return node;
	}

	// Had to check for pipe AFTER '!' because '!' cheange the value of a whole pipe chain	
	tok_current = tok_start;
	while (tok_current <= tok_end) {
		if (tokens[tok_current]->type == t_pipe) {
			node->type = p_pipe;
			node->args = calloc(2, sizeof(char*));
			(node->args)[0] = "|";

			node->left = parsing(tokens, tok_start, tok_current - 1);
			node->rigth = parsing(tokens, tok_current + 1, tok_end);
			return node;
		}
		tok_current++;
	}

	// Check for io redirection
	tok_current = tok_start;
	while (tok_current <= tok_end) {
		// <, >, <>, >>
		if (is_io_par(tokens[tok_current]->type)) {
			node->args = calloc(4, sizeof(char*));
			// Setup parametric values
			setup_io_pars(node, tokens[tok_current]->type);
			(node->args)[1] = tokens[tok_current + 1]->value; // Next token is the file target
			int offset = 0;
			// Check if there is an ioarg
			if (tokens[tok_current - 1]->type == t_ioarg) {
				(node->args)[2] = tokens[tok_current - 1]->value;
				offset = 1;
			}
			else
				(node->args)[2] = NULL;

			node->left = parsing(tokens, tok_start, tok_current - 1 - offset);
			return node;
		}
		tok_current++;
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

