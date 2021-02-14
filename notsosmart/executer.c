#include "headers.h"

int is_builtin(char* cmd) {
	for (int i = 0; i < builtin_n; i++) {
		if (strcmp(cmd, lookup_funct[i]) == 0)
			return i;
	}
	return -1;
}

int execute_piped_cmd(char** cmd_args, int io, int* pipe) {
	// Check if command is a builtin function; if true run it
	int bi_ind = is_builtin(cmd_args[0]);
	if (bi_ind != -1)
		return builtin_funct[bi_ind](cmd_args);

	int pid;
	int status;
	pid = fork();
	if (pid == 0) {
		dup2(pipe[io], io);
		close(pipe[io]);
		close(pipe[1 - io]);
		execvp(cmd_args[0], cmd_args);
		exit(EXIT_FAILURE);
	}
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
		//printf("Exit code [%d]\n", status); // DEBUG EXIT CODE
		return status;
	}
}


int execute_shubshell(pn* root) {
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0) {
		int e = execute(root);
		//printf("Exit code e [%d]\n", e); // DEBUG EXIT CODE
		exit(e);
	}
	else {
		waitpid(pid, &status, 0);
		//printf("Exit code sub [%d]\n", status); // DEBUG EXIT CODE
		return status;
	}
}

int execute(pn* root) {
	if (root == NULL)
		return 0;
	else if (root->type == p_arg) {
		// Standard execution
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
	else if (root->type == p_bang) {
		return execute(root->left) != 0 ? 0 : 1;
	}
	else if (root->type == p_pipe) {
		// pipe_ends[0]: standard input
		// pipe_ends[1]: standard output
		int pipe_ends[2];
		int pipe_res = pipe(pipe_ends);

		int cmd_ret1 = execute_piped_cmd(root->left->args, STDOUT_FILENO, pipe_ends);
		int cmd_ret2 = execute_piped_cmd(root->rigth->args, STDIN_FILENO, pipe_ends);
		waitpid(-1, NULL, 0);
		return cmd_ret2;
	}

	return -1;
}