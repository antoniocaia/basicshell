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
		// DEBUGG
		// EXIT CODE
		//printf("Exit code [%d]\n", status);
		return status;
	}
}


int execute_shubshell(pn* root) {
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0) {
		int e = execute(root);
	}
	else {
		waitpid(pid, &status, 0);
		// DEBUGG
		// EXIT CODE
		//printf("Exit code sub [%d]\n", status);
		return status;
	}
}

int execute(pn* root) {
	if (root == NULL)
		return 0;
	else if (root->type == p_arg) {
		return execute_cmd(root->args);
	}
	else if (root->type == p_subshell) {
		// Run a subshell 
		return execute_shubshell(root->left);
	}
	else if (root->type == p_separator) {
		// ';' do nothing
		execute(root->left);
		return execute(root->rigth);
	}
	else if (root->type == p_and) {
		// &&: if first cmd true then second cmd run
		if (execute(root->left) == 0)
			return execute(root->rigth);

		return 1;
	}
	else if (root->type == p_or) {
		// ||: if first cmd false then second cmd run
		if (execute(root->left) != 0)
			return execute(root->rigth);

		return 0;
	}

	return -1;
}