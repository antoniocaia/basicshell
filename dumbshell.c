#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>

#define COMMAND_TOKENS 16

// ---------------- STATUSES

typedef enum
{
	ERROR_INPUT,
	VALID_INPUT
} InputStatus;

typedef enum
{
	ERROR_PARSE,
	VALID_PARSE
} ParseStatus;

typedef enum
{
	INVALID_COMMAND,
	INVALID_OPTION,
	VALID_COMMAND,
	ERROR_COMMAND
} CommandStatus;

// ---------------- BUILTIN FUNCTION

CommandStatus cdd(char **args);
CommandStatus helpd(char **args);
CommandStatus exitd(char **args);
CommandStatus lsd(char **args);

char *lookup_funct[] = {
	"cdd",
	"helpd",
	"exitd"};

CommandStatus (*builtin_funct[])(char **) = {
	&cdd,
	&helpd,
	&exitd};

int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

CommandStatus cdd(char **args)
{
	if (chdir(args[1]) == -1)
	{
		return INVALID_OPTION;
	}

	return VALID_COMMAND;
}

CommandStatus helpd(char **args)
{
	printf("Builtin command:\n");
	for (int i = 0; i < builtin_n; i++)
	{
		printf("- %s\n", lookup_funct[i]);
	}
	return VALID_COMMAND;
}

CommandStatus exitd(char **args)
{
	exit(EXIT_SUCCESS);
	return ERROR_COMMAND;
}

// ---------------- LIFE-CYCLE

// Reads from stdin one line containing command + args
InputStatus read_input(char **p_buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(p_buffer, &size, stdin);

	if (bytes_read <= 0)
	{
		return ERROR_INPUT;
	}

	if ((*p_buffer)[bytes_read - 1] == '\n')
	{
		(*p_buffer)[bytes_read - 1] = '\0';
		bytes_read--;
	}
	return VALID_INPUT;
}

// Parse the buffer, putting the command in p_command and args in p_args
ParseStatus parse(char *p_buffer, char **p_args)
{
	char *separator = " ";
	int args_ind = 0;

	p_args[0] = strtok(p_buffer, separator);
	if (p_args[0] == NULL)
		return ERROR_PARSE;

	do
	{
		args_ind++;
		p_args[args_ind] = strtok(NULL, separator);
	} while (p_args[args_ind] != NULL);

	return VALID_PARSE;
}

// Execute external programs creating a child process
CommandStatus new_process(char **p_args)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(p_args[0], p_args) == -1)
		{
			return ERROR_COMMAND;
		}
	}
	else if (pid < 0)
	{
		return ERROR_COMMAND;
	}

	wait(NULL);
	return VALID_COMMAND;
}

// Check if a command is a builtin or external, and lunchs it
CommandStatus execute(char **p_args)
{
	bool external = true;
	for (int i = 0; i < builtin_n; i++)
	{
		if (strcmp(p_args[0], lookup_funct[i]) == 0)
		{
			return builtin_funct[i](p_args);
		}
	}

	if (external)
		return new_process(p_args);

	return VALID_COMMAND;
}

void life_cycle()
{
	while (true)
	{
		printf("> ");
		char *p_buffer;

		switch (read_input(&p_buffer))
		{
		case (VALID_INPUT):
			break;
		case (ERROR_INPUT):
			printf("Stdin error\n");
			continue;
		}

		char **p_args = malloc(sizeof(char *) * COMMAND_TOKENS);
		switch (parse(p_buffer, p_args))
		{
		case (VALID_PARSE):
			break;
		case (ERROR_PARSE):
			printf("...?\n");
			continue;
		}

		switch (execute(p_args))
		{
		case (VALID_COMMAND):
			break;
		case (INVALID_COMMAND):
			printf("Invalid command\n");
			continue;
		case (ERROR_COMMAND):
			printf("Error command\n");
			continue;
		case (INVALID_OPTION):
			printf("Invalid options\n");
			continue;
		}

		free(p_buffer);
	}
}

int main(int argc, char **argv)
{
	// Init, configuration

	// Read, parse, execute
	life_cycle();

	// Shut down, free memory etc

	return 0;
}
