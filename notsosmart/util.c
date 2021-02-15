#include "headers.h"

bool is_io_par(int t) {
	return t == t_ldm
		|| t == t_rdm
		|| t == t_lrdm
		|| t == t_rrdm;
}

bool is_io_ex(int t) {
	return t == p_ldm
		|| t == p_rdm
		|| t == p_lrdm
		|| t == p_rrdm;
}

bool is_string(char* buffer, int bf_end) {
	return isalpha(buffer[bf_end])
		|| isdigit(buffer[bf_end])
		|| buffer[bf_end] == '_'
		|| buffer[bf_end] == '-'
		|| buffer[bf_end] == '.'
		|| buffer[bf_end] == '/';
}

// DEBUG
// TEST TOKEN
void print_tokens(tok** tl) {
	int i = 0;
	while (tl[i] != NULL) {
		printf("[%s][%d]   ", tl[i]->value, tl[i]->type);
		i++;
	}
	printf("\n\n");
}

// DEBUG
// TEST NODE
void print_pars(pn* root) {
	if (root == NULL) return;
	printf("Type [%d]\n", root->type);
	int i = 0;
	while ((root->args)[i] != NULL) {
		printf("[%s]", (root->args)[i]);
		i++;
	}
	printf("\n\n");
	print_pars(root->left);
	print_pars(root->rigth);
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