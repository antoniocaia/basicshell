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
#include <string.h>

/*
-------------------------------------------------------
|                   Custom terminal                   |
-------------------------------------------------------
*/

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

/*
-----------------------------------------------
|                   Builtin                   |
-----------------------------------------------
*/

void b_cd(char **args);
void b_help(char **args);
void b_ex(char **args);
void b_exec(char **args);

char *lookup_funct[] = {
	"cd",
	"help",
	"ex",
	"exec"};

void (*builtin_funct[])(char **) = {
	&b_cd,
	&b_help,
	&b_ex,
	&b_exec};

int builtin_n = sizeof(lookup_funct) / sizeof(lookup_funct[0]);

void b_cd(char **args)
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

void b_help(char **args)
{
	printf("Builtin command:\n");
	for (int i = 0; i < builtin_n; i++)
	{
		printf("- %s\n", lookup_funct[i]);
	}
}

void b_ex(char **args)
{
	exit(EXIT_SUCCESS);
}

void b_exec(char **args)
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

/*
-----------------------------------------------
|                   Generic                   |
-----------------------------------------------
*/

bool check_for_subshell(char *buffer, char **start, char **end)
{
	*start = strchr(buffer, '(');
	*end = strchr(buffer, ')');

	if (*start != NULL && *end != NULL)
		return true;

	return false;
}

char *find_min_pointer(char **pointers, int n)
{
	char *min = (char *)-1;
	for (int i = 0; i < n; i++)
	{	//printf("p %p \n", pointers[0]);
		if (pointers[i] < min)
			min = pointers[i];
	}
	return min;
}

int get_next_operator(char *buffer, char **operators, int num_of_op, char **min)
{
	
	// Init
	operators[0] = strstr(buffer, ";");
	operators[1] = strstr(buffer, "&&");
	operators[2] = strstr(buffer, "||");

	
	for (int i = 0; i < num_of_op; i++)
	{
		//printf("%p\n", operators[i]);
		if (operators[i] == NULL)
			operators[i] = (char *)-1;
	}
	
	// exit or go on
	int i = 0;
	while (operators[i] == (char *)-1 && i < num_of_op)
	{
		i++;
	}
	if (i == num_of_op)
		return -1;

	
	*min = find_min_pointer(operators, num_of_op);
	//printf("MIN: %s\n", *min);
	return 0;
}

/*
--------------------------------------------------
|                   Life-cycle                   |
--------------------------------------------------
*/

int read_input(char **buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(buffer, &size, stdin);
	if (bytes_read == -1)
	{
		// EOF
		exit(EXIT_SUCCESS);
	}
	else if (bytes_read == 1)
	{
		// Blank line
		return -1;
	}

	if ((*buffer)[bytes_read - 1] == '\n')
	{
		(*buffer)[bytes_read - 1] = '\0';
		bytes_read--;
	}
	return 0;
}

int string_to_tokens(char *buffer, char **args, char *separator)
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
		index++;
		args[index] = strtok(NULL, separator);
	} while (args[index] != NULL);
	return 0;
}

void standard_execute(char **args)
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
			err_status = true;
			exit(EXIT_FAILURE);
		}
		else
		{
			waitpid(pid, &status, 0);
			if (status != 0)
				err_status = true;
			else
				err_status = false;
		}
	}
}

void subshell_execute(char **args)
{
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		standard_execute(args);
		exit(EXIT_FAILURE);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (status != 0)
			err_status = true;
		else
			err_status = false;
	}
}

bool check_subshell(char *next_command)
{
	return (next_command[0] == '(' && next_command[strlen(next_command) - 1] == ')');
}

void get_next_command(char **buffer, char **next_command, char *end)
{
	*next_command = calloc(end - *buffer + 1, sizeof(char));
	strncpy(*next_command, *buffer, end - *buffer);
	*buffer = end + 2;
}

void execute_next_command(char *next_command)
{
	char **p_args = calloc(16, sizeof(char *));
	//printf("nex_t_com %s", next_command);
	string_to_tokens(next_command, p_args, " ");
	standard_execute(p_args);
}

void check_operator_code_and_run(int current_operator_code, char *next_command)
{
	//printf("%d \n", current_operator_code);
	if (current_operator_code >= 0 && err_status == 0)
		execute_next_command(next_command);
	else if (current_operator_code <= 0 && err_status == 1)
		execute_next_command(next_command);
}

int get_next_operator_code(char **operators, char *min)
{
	if (min == operators[0])
		return 0;
	else if (min == operators[1])
		return 1;
	else if (min == operators[2])
		return -1;
}

void parse_line(char *buffer, int operator_code)
{
	// 0 : ;
	// 1 : &&
	// -1 : ||
	int next_operator_code = 0;

	int num_of_op = 3;
	char *operators[num_of_op];
	char *brack[2];

	brack[0] = strchr(buffer, '(');
	brack[1] = strchr(buffer, ')');
	if (brack[0] == NULL)
		brack[0] = (char *)-1;
	if (brack[1] == NULL)
		brack[1] = (char *)-1;

	//MEM
	// printf("Brack: %p   %p\n", brack[0], brack[1]);
	// for (int i = 0; i < num_of_op; i++)
	// 	printf("%p ", operators[i]);
	// printf("\n");

	//INPUT
	//printf("input BUF: %s   Oper: %d\n", buffer, operator_code);

	char *min;
	if (get_next_operator(buffer, operators, num_of_op, &min) < 0 && brack[0] == (char *)-1)
	{
		//LOG
		//printf("Processing simple command\n");

		char *next_command;
		//get_next_command(&buffer, &next_command, buffer + strlen(buffer));
		check_operator_code_and_run(operator_code, buffer);
		return;
	}

	if (brack[0] < min)
	{
		//LOG
		//printf("Processing brackets \n");

		char *tmp_buffer = calloc(255, sizeof(char));
		strncpy(tmp_buffer, buffer + 1, brack[1] - buffer - 1);
		tmp_buffer[strlen(tmp_buffer)] = '\0';
		buffer = brack[1];

		//MEM
		//printf("( ): %s\n", tmp_buffer);
		//printf("BUFFER: %s\n", buffer);

		// process subshell
		parse_line(tmp_buffer, operator_code);

		// rest of the string
		char *connector;
		get_next_operator(buffer, operators, num_of_op, &connector);
		buffer = buffer + 2;

		//MEM
		//printf("UP_BUFFER: %s  con: %s\n", buffer, connector);
		//parse_line(buffer, get_next_operator_code(operators, connector));
	}
	else
	{
		//LOG
		// printf("Processing composite \n");
		// printf(" Actual input of min? %s\n", min);
		next_operator_code = get_next_operator_code(operators, min);

		//printf("new op coede: %d\n", next_operator_code);
		char *next_command;
		get_next_command(&buffer, &next_command, min);
		//printf("NEX command: %s\n", next_command);

		check_operator_code_and_run(operator_code, next_command);

		buffer = min + 2;
		//printf("\nNEW_BUFF : %s\n", buffer);
		parse_line(buffer, next_operator_code);
	}
}

/*
--------------------------------------------
|                   Main                   |
--------------------------------------------
*/

int main(int argc, char **argv)
{
	// Enviroment
	char **path_args = malloc(sizeof(char *) * 32);
	char path_var[255];
	strcpy(path_var, getenv("PATH"));
	string_to_tokens(path_var, path_args, ":");

	// Look
	update_current_dir_path();
	update_battery_level();
	update_time();

	// Loop
	while (true)
	{
		terminal();
		if (feof(stdin))
			exit(EXIT_SUCCESS);

		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == -1)
			continue;
		parse_line(buffer, 0);
	}

	exit(EXIT_SUCCESS);
}