#include "headers.h"

int is_builtin(char* cmd) {
	for (int i = 0; i < builtin_n; i++) {
		if (strcmp(cmd, lookup_funct[i]) == 0)
			return i;
	}
	return -1;
}


int execute(char** args) {
	// Check if command is a builtin function; if true run it
	int bi_ind = is_builtin(args[0]);
	if (bi_ind != -1)
		return builtin_funct[bi_ind](args);

	// Standard child-parent fork
	int pid;
	int status;
	pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
		exit(EXIT_FAILURE);
	}
	else {
		waitpid(pid, &status, 0);
		return status;
	}
}