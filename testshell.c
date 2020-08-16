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

// Custom terminal

#define RED "\033[0;31m"
#define BLUE "\033[0;34m"
#define LIGTH_BLUE "\033[1;34m"
#define CYAN "\033[0;36m"
#define LIGTH_CYAN "\033[1;36m"
#define NC "\033[0m"

#define COLOR_SET LIGTH_BLUE
char current_path[64];
char *main_color;

//char *notification_color;

int number_of_elements(char *arr)
{
	int i = 0;
	while (arr[i] != NULL)
		i++;
	return i;
}

void update_current_dir_path()
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

void terminal()
{
	printf("%s%s", main_color, current_path);
	printf("> $ %s", NC);
}

// Builtin

void cd(char **args);
void help(char **args);
void ex(char **args);

char *lookup_funct[] = {
	"cd",
	"help",
	"ex"};

void (*builtin_funct[])(char **) = {
	&cd,
	&help,
	&ex};

int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

void cd(char **args)
{
	if (chdir(args[1]) == -1)
	{
		// CHECK
	}
	update_current_dir_path();
}

void help(char **args)
{
	printf("Builtin command:\n");
	for (int i = 0; i < builtin_n; i++)
	{
		printf("- %s\n", lookup_funct[i]);
	}
}

void ex(char **args)
{
	exit(EXIT_SUCCESS);
}

// LIFECYCLE

void read_input(char **buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(buffer, &size, stdin);

	if (bytes_read <= 0)
	{
		printf("ERROR");
	}

	if ((*buffer)[bytes_read - 1] == '\n')
	{
		(*buffer)[bytes_read - 1] = '\0';
		bytes_read--;
	}
}

void parse(char *buffer, char **args, char *separator)
{
	int index = 0;

	args[index] = strtok(buffer, separator);
	if (args[index] == NULL)
	{
		//CHECK
		printf("ERROR");
	}

	do
	{
		printf("%s\n", args[index]);
		index++;
		args[index] = strtok(NULL, separator);
	} while (args[index] != NULL);
}


void execute(char **args)
{
	bool external = true;
	for (int i = 0; i < builtin_n; i++)
	{
		if (strcmp(args[0], lookup_funct[i]) == 0)
		{
			external = false;
			builtin_funct[i](args);
		}
	}

	if (external)
	{
		pid_t pid;
		int status;
		pid = fork();
		if (pid == 0)
			execvp(args[0], args);

		wait(&status);

		if (status != 0)
			main_color = RED;
		else
			main_color = COLOR_SET;
	}
}

int main(int argc, char **argv)
{
	// Enviroment
	char *path_var = getenv("PATH");
	char **path_args = malloc(sizeof(char *) * 16);
	//parse(path_var, path_args, ":");

	// Look
	update_current_dir_path();
	main_color = COLOR_SET;

	// Loop
	while (true)
	{
		terminal();
		char *buffer;
		read_input(&buffer);
		char **p_args = malloc(sizeof(char *) * 16);
		parse(buffer, p_args, " ");
		execute(p_args);
	}

	exit(EXIT_SUCCESS);
}