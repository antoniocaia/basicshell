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

#define NO_COLOR "\033[0m"
#define RED "\033[0;31m"
#define MAIN_COLOR "\033[1;34m"
#define NOTIF_COLOR "\033[1;36m"
#define TIME_COLOR "\033[0;36m"
char *main_color = MAIN_COLOR;
char *notification_color = NOTIF_COLOR;

#define BATTERY_ALLERT_VALUE 30
int current_battery;

char current_time[6];
char current_path[64];
bool err_status = false;

void update_time()
{
	FILE *fp = popen("date +'%I:%M'", "r");
	if (fp == NULL)
	{
		perror("no date\n");
		return;
	}
	fgets(current_time, sizeof(current_time), fp);
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

void update_current_dir_path()
{
	FILE *fp = popen("pwd", "r");
	if (fp == NULL)
	{
		perror("Failed to call pwd with popen");
	}
	fgets(current_path, sizeof(current_path), fp);
	current_path[strlen(current_path) - 1] = '\0';
	pclose(fp);
}

void terminal()
{
	if (err_status)
		notification_color = RED;
	else
		notification_color = NOTIF_COLOR;

	printf("%s%s ", MAIN_COLOR, current_path);
	printf("%s%s ", TIME_COLOR, current_time);
	if (update_battery_level())
	{
		printf("%s%d ", RED, current_battery);
	}
	printf("\n");
	printf("%s> $ %s", notification_color, NO_COLOR);
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
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Tried to chdir into %s", args[1]);
		perror(buffer);
		err_status = true;
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

// Life-cycle

int read_input(char **buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(buffer, &size, stdin);
	if (bytes_read <= 0)
	{
		// EOF
		exit(EXIT_SUCCESS);
	}
	else if (bytes_read == 1)
	{
		// No input
		return -1;
	}

	if ((*buffer)[bytes_read - 1] == '\n')
	{
		(*buffer)[bytes_read - 1] = '\0';
		bytes_read--;
	}
	return 0;
}

int parse(char *buffer, char **args, char *separator)
{
	int index = 0;

	args[index] = strtok(buffer, separator);
	if (args[index] == NULL)
	{
		printf("Error parsing with  '%s'", separator);
		return -1;
	}

	do
	{
		//printf("'%s'\n", args[index]);
		index++;
		args[index] = strtok(NULL, separator);
	} while (args[index] != NULL);
	return 0;
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
		{
			execvp(args[0], args);
			perror("ERROR RUNNING CHILD");
		}

		waitpid(pid, &status, 0);
		if (status != 0)
			err_status = true;
		else
			err_status = false;
	}
}

char *parse_next_command(char **buffer, char *end)
{
	char *next_command = malloc(256);
	strncpy(next_command, *buffer, end - *buffer);
	*buffer = end + 2;
	printf("%s\n", next_command);
	return next_command;
}

void execute_next_command(char *next_command)
{
	char **p_args = malloc(sizeof(char *) * 16);
	parse(next_command, p_args, " ");
	execute(p_args);
}

//char ** commands, char **operators, char *separator
void parse_multiple_commands(char *buffer)
{
	// 0 : ;
	// 1 : &&
	// -1 : ||
	int current_operator = 0;
	int next_operator = 0;

	char *p_seq;
	char *p_and;
	char *p_or;

	int c = 0;
	while (true)
	{
		p_seq = strstr(buffer, ";");
		p_and = strstr(buffer, "&&");
		p_or = strstr(buffer, "||");

		//printf("---\n%p\n%p\n%p\n", p_seq, p_and, p_or);

		if (p_seq == NULL)
			p_seq = (char *)-1;
		if (p_and == NULL)
			p_and = (char *)-1;
		if (p_or == NULL)
			p_or = (char *)-1;

		if (p_seq == (char *)-1 && p_and == (char *)-1 && p_or == (char *)-1)
			break;

		char *p_end_command;
		if (p_seq < p_and && p_seq < p_or)
		{
			next_operator = 0;
			p_end_command = p_seq;
		}
		else if (p_and < p_seq && p_and < p_or)
		{
			next_operator = 1;
			p_end_command = p_and;
		}
		else if (p_or < p_and && p_or < p_seq)
		{
			next_operator = -1;
			p_end_command = p_or;
		}

		char *next_command = parse_next_command(&buffer, p_end_command);
		if (current_operator >= 0 && err_status == 0)
			execute_next_command(next_command);
		else if (current_operator <= 0 && err_status == 1)
			execute_next_command(next_command);

		current_operator = next_operator;
	}

	char *next_command = parse_next_command(&buffer, strchr(buffer, '\0'));
	if (current_operator >= 0 && err_status == 0)
		execute_next_command(next_command);
	else if (current_operator <= 0 && err_status == 1)
		execute_next_command(next_command);
}

// if (p_seq != NULL)
// {
// 	p_seq[0] = ' ';
// }
// if (p_and != NULL)
// {
// 	p_and[0] = ' ';
// 	p_and[1] = ' ';
// }
// if (p_or != NULL)
// {
// 	p_or[0] = ' ';
// 	p_or[1] = ' ';
// }

int main(int argc, char **argv)
{
	// Enviroment
	char **path_args = malloc(sizeof(char *) * 32);
	char path_var[255];
	strcpy(path_var, getenv("PATH"));
	parse(path_var, path_args, ":");

	// Look
	update_current_dir_path();
	update_battery_level();
	update_time();

	// Loop
	// while (true)
	// {
	// 	terminal();
	// 	char *buffer;
	// 	int read_res = read_input(&buffer);
	// 	if (read_res == -1)
	// 		continue;

	// 	int index = 0;
	// 	int last_pos = 0;
	// 	int status = 0;
	// 	while (buffer[index] != '\0')
	// 	{
	// 		while (buffer[index] != ';' && buffer[index] != '|' && buffer[index] != '&' && buffer[index] != '\0')
	// 		{
	// 			index++;
	// 			if (buffer[index] == ';') status = 0;
	// 			if (buffer[index] == '&&') status = 0;
	// 			if (buffer[index] == ';') status = 0;
	// 		}

	// 		if (status == 0)
	// 		{
	// 			// ; run anything
	// 		}
	// 		else if (status == 1)
	// 		{
	// 			// run && only status is 0
	// 			if(!err_status){
	// 				// DO NEXT
	// 			}
	// 		}
	// 		else if (status == -1)
	// 		{
	// 			if(err_status){
	// 				// run || only status is -1
	// 				// DO NEXT
	// 			}
	// 		}

	// 		if (buffer[index] == ';' || buffer[index] == '\0')
	// 		{
	// 			int size = index - last_pos;
	// 			char subbuff[size];
	// 			memcpy(subbuff, &buffer[last_pos], size);
	// 			subbuff[size] = '\0';
	// 			//printf("%s\n", subbuff);

	// 			char **p_args = malloc(sizeof(char *) * 16);
	// 			parse(subbuff, p_args, " ");
	// 			execute(p_args);

	// 			index = index + 1;
	// 			last_pos = index;
	// 		}
	// 	}
	// Loop
	while (true)
	{
		terminal();
		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == -1)
			continue;
		parse_multiple_commands(buffer);
		char **p_args = malloc(sizeof(char *) * 16);
		//parse(buffer, p_args, " ");
		//execute(p_args);
	}

	exit(EXIT_SUCCESS);
}