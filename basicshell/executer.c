#include "headers.h"

int is_builtin(char* cmd) {
	for (int i = 0; i < builtin_n; i++) {
		if (strcmp(cmd, lookup_funct[i]) == 0)
			return i;
	}
	return -1;
}

// Run a standard cmd + args, no io redirection
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
		int e = execvp(cmd_args[0], cmd_args);
		printf("Errno [%s] with command [%s] (%d)\n", strerror(errno), cmd_args[0], e);
		exit(EXIT_FAILURE);
	}
	else {
		waitpid(pid, &status, 0);
		//printf("Exit code [%d]\n", status); 			// DEBUG EXIT CODE
		return status;
	}
}

int execute_pipe(pn* root, int io, int* pipe_ends) {
	int pid;
	int status;
	pid = fork();
	if (pid == 0) {
		dup2(pipe_ends[io], io);
		close(pipe_ends[1 - io]);	// Close the unused end
		int e = execute(root);
		exit(e);
	}
	else {
		waitpid(pid, &status, 0);
		return status;
	}
}

int execute_shubshell(pn* root) {
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0) {
		int e = execute(root);
		//printf("Exit code e [%d]\n", e);				// DEBUG EXIT CODE
		exit(e);
	}
	else {
		waitpid(pid, &status, 0);
		//printf("Exit code sub [%d]\n", status);		// DEBUG EXIT CODE
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
		return execute(root->rigth) != 0 ? 0 : 1;
	}
	else if (root->type == p_pipe) {
		// |: redirect stdin and stdout using pipe. Not sure about multiple pipe yet, or more complex cmd
		// pipe_ends[0]: standard input   pipe_ends[1]: standard output
		int pipe_ends[2];
		int p = pipe(pipe_ends);
		execute_pipe(root->left, STDOUT_FILENO, pipe_ends);
		// Close the pipe output channel, so that the second cmd doesn't wait for more data
		close(pipe_ends[STDOUT_FILENO]);
		return execute_pipe(root->rigth, STDIN_FILENO, pipe_ends);
	}
	else if (is_io_ex(root->type)) {
		int file_fd;
		int replace_this_fd;

		if (root->type == p_rdm) {
			file_fd = open(root->args[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
			replace_this_fd = 1;
		}
		else if (root->type == p_ldm) {
			file_fd = open(root->args[1], O_RDONLY, 0666);
			replace_this_fd = 0;
		}
		else if (root->type == p_lrdm) {
			file_fd = open(root->args[1], O_CREAT | O_RDWR, 0666);
			replace_this_fd = 0;
		}
		else if (root->type == p_rrdm) {
			file_fd = open(root->args[1], O_CREAT | O_APPEND | O_WRONLY, 0666);
			replace_this_fd = 1;
		}

		// Handle wrong file path
		if (file_fd == -1) {
			printf("Errno: [%s]. Can't open file [%s]", strerror(errno), root->args[1]);
			return -1;
		}

		// If a different fd is specified, set it, otherwise use the default one
		replace_this_fd = root->args[2] == NULL ? replace_this_fd : atoi(root->args[2]);
		// Save stdout/stdin so we can restore it later
		int save_std = dup(replace_this_fd);
		// Switch
		dup2(file_fd, replace_this_fd);
		close(file_fd);
		// Go on with program execution
		int exec_res = execute(root->left);
		// Restore stdout/stdin
		dup2(save_std, replace_this_fd);
		close(save_std);
		return exec_res;
	}
	else if (root->type == p_endrdm || root->type == p_ldmend) {
		// Default fd
		int replace_this_fd = root->type == p_endrdm ? 1 : 0;	// Se non è zuppa è panbagnato
		// Check if a different fd is specified instead of the default one
		replace_this_fd = root->args[2] == NULL ? replace_this_fd : atoi(root->args[2]);
		// Check if the input arg is - (close)
		if (root->args[1] == "-") {
			close(replace_this_fd);
		}	// Otherwise we replace the target fd with the arg
		else {
			int new_fd = atoi(root->args[1]);
			dup2(new_fd, replace_this_fd);
			close(new_fd);
		}
		// Go on with program execution
		int exec_res1 = execute(root->left);
		int exec_res2 = execute(root->rigth);

		return exec_res1;
	}
	else {
		printf("No handler for the current type: [%d][%s]\n", root->type, root->args[0]);
	}

	return -1;
}

//./list-fds 0<&-