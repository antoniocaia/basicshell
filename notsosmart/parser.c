#include "headers.h"

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