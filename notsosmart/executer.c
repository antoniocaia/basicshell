#include "headers.h"

int is_builtin(char* cmd) {
	for (int i = 0; i < builtin_n; i++) {
		if (strcmp(cmd, lookup_funct[i]) == 0)
			return i;
	}
	return -1;
}

int execute_cmd(char** cmd_args) {
	// Check if command is a builtin function; if true run it
	int bi_ind = is_builtin(cmd_args[0]);
	if (bi_ind != -1)
		return builtin_funct[bi_ind](cmd_args);

	// Standard child-parent fork
	int pid;
	int status;
	pid = fork();
	if (pid == 0) {
		execvp(cmd_args[0], cmd_args);
		exit(EXIT_FAILURE);
	}
	else {
		waitpid(pid, &status, 0);
		return status;	
	}
}

int execute(pn* root) {
	if (root == NULL)
		return 0;
	else if (root->type == p_arg) {
		return execute_cmd(root->args);
		//execute(root->left);
	}
	else if (root->type == p_separator) {
		// ';' do nothing
		execute(root->left);
		execute(root->rigth);
	}
	return -1;
}