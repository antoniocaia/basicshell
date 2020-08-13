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
#define MAX_PATH_LENGTH 64
#define BATTERY_ALLERT_VALUE 30

#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define LIGTH_BLUE "\033[1;34m"
#define CYAN "\033[0;36m"
#define LIGTH_CYAN "\033[1;36m"
#define NC "\033[0m"

char current_path[MAX_PATH_LENGTH];
int current_battery;
char current_time[6];

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

// ---------------- CUSTOMIZE

int number_of_elements(char *arr)
{
	int i = 0;
	while (arr[i] != NULL)
		i++;
	return i;
}

void update_current_path()
{
	FILE *fp = popen("pwd", "r");
	if (fp == NULL)
	{
		perror("no pwd\n");
		return;
	}
	fgets(current_path, sizeof(current_path), fp);
	int last_pos = number_of_elements(current_path) - 1;
	current_path[last_pos] = '\0';
	pclose(fp);
}

void update_time()
{
	FILE *fp = popen("date +'%I:%M'", "r");
	if (fp == NULL)
	{
		perror("no date\n");
		return;
	}
	fgets(current_time, sizeof(current_time), fp);
	//int last_pos = number_of_elements(current_time) - 1;
	//current_time[last_pos] = '\0';
}

bool update_battery_level()
{
	FILE *fp = fopen("/sys/class/power_supply/BAT0/capacity", "rt");
	if (fp == NULL)
	{
		perror("can't find /sys/class/power_supply/BAT0/capacity\n");
		return false;
	}
	fscanf(fp, "%d", &current_battery);
	fclose(fp);
	if (current_battery <= BATTERY_ALLERT_VALUE)
	{
		return true;
	}

	return false;
}

// Prompr displaycd
void prompt()
{
	char *main_color = LIGTH_BLUE;
	printf("%sDUMB:", CYAN);
	printf("%s%s ", main_color, current_path);
	printf("%s%s ", LIGTH_CYAN, current_time);
	if (update_battery_level())
	{
		printf("%s!%d! ", RED, current_battery);
	}
	printf("%s> %s", main_color, NC);
}

// ---------------- BUILTIN FUNCTION

CommandStatus cd(char **args);
CommandStatus helpd(char **args);
CommandStatus exitd(char **args);
CommandStatus lsd(char **args);

char *lookup_funct[] = {
	"cd",
	"helpd",
	"exitd"};

CommandStatus (*builtin_funct[])(char **) = {
	&cd,
	&helpd,
	&exitd};

int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

CommandStatus cd(char **args)
{
	if (chdir(args[1]) == -1)
	{
		return INVALID_OPTION;
	}
	update_current_path();
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
		prompt();
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
	update_battery_level();
	update_current_path();
	update_time();
	// Read, parse, execute
	life_cycle();

	// Shut down, free memory etc

	return 0;
}
