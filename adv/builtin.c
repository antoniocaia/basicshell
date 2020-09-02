#include "headers.h"

int b_cd(char **args)
{
	if (chdir(args[1]) == -1)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Tried to chdir into %s", args[1]);
		perror(buffer);
		has_exec_failed = true;
		return 1;
	}
	//update_current_dir_path();
	return 0;
}

int b_help(char **args)
{
	printf("Builtin command:\n");
	for (int i = 0; i < builtin_n; i++)
	{
		printf("- %s\n", lookup_funct[i]);
	}
	return 0;
}

int b_ex(char **args)
{
	if (args[1] != NULL)
	{
		int exit_value = atoi(args[1]);
		exit(exit_value);
	}

	exit(EXIT_SUCCESS);
}

int b_exec(char **args)
{
	args = &args[1];
	int exit_code = execvp(args[0], args);
	if (exit_code == -1)
	{
		exit(EXIT_FAILURE);
	}
	else
	{
		exit(EXIT_SUCCESS);
	}
}