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
#include <fcntl.h>

#define INPUT_PIPE 1
#define OUTPUT_PIPE 0
#define STDIN_FD 0
#define STDOUT_FD 1

bool failed_exec = false;

int tokenizer(char *buffer, char **args, char *separator);
int standard_execute(char **args);
void execute_line(char **line_tokens);

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
	if (failed_exec)
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
	"exit",
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
		failed_exec = true;
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
	int exit_value = atoi(args[1]);
	exit(exit_value);
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
------------------------------------------------------
|                   Parse-specific                   |
------------------------------------------------------
*/

// Return the smaller address of a list of operators (the "next" one in the line to be processed)
char *find_smaller_p_address(char **pointers, int n)
{
	char *min = (char *)-1;
	for (int i = 0; i < n; i++)
	{
		if (pointers[i] < min)
			min = pointers[i];
	}
	return min;
}

// Check if there are operators in the string (buffer)
// Return -1 if no operator is found, otherwise the address of the next one
int get_next_operator(char *buffer, char **operators, int num_of_op, char **next_p)
{
	// Init
	operators[0] = strstr(buffer, ";");
	operators[1] = strstr(buffer, "&&");
	operators[2] = strstr(buffer, "||");
	operators[3] = strstr(buffer, "|");

	for (int i = 0; i < num_of_op; i++)
	{
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

	*next_p = find_smaller_p_address(operators, num_of_op);
	return 0;
}

// Extrapolates command, remove OPERATOR
void get_next_command(char **buffer, char **next_command, char *end)
{
	*next_command = calloc(end - *buffer + 1, sizeof(char));
	strncpy(*next_command, *buffer, end - *buffer);

	if (end[0] == ';')
		*buffer = end + 1;
	else
		*buffer = end + 2;
}

// Tokenize and run command
void execute_next_command(char *next_command)
{
	char **p_args = calloc(16, sizeof(char *));
	tokenizer(next_command, p_args, " ");
	standard_execute(p_args);
}

// Check if the operator and the exit code of last command allows this command to run
void check_code_with_err_then_run(int current_operator_code, char *next_command)
{
	if (current_operator_code >= 0 && failed_exec == 0)
		execute_next_command(next_command);
	else if (current_operator_code <= 0 && failed_exec == 1)
		execute_next_command(next_command);
}

int get_next_operator_code(char **operators, char *next_p)
{
	// [0  ;]      [1  &&]      [-1  ||]
	if (next_p == operators[0])
		return 0;
	else if (next_p == operators[1])
		return 1;
	else if (next_p == operators[2])
		return -1;
}

void check_for_brackets(char *buffer, char **brack)
{
	brack[0] = strchr(buffer, '(');
	brack[1] = strchr(buffer, ')');
	if (brack[0] == NULL)
		brack[0] = (char *)-1;
	if (brack[1] == NULL)
		brack[1] = (char *)-1;
}

int check_line_continuation(char *buffer)
{
	if (buffer[strlen(buffer) - 1] == '\\')
		return -1;
	else if ((buffer[strlen(buffer) - 1] == '&' && buffer[strlen(buffer) - 2] == '&') || (buffer[strlen(buffer) - 1] == '|' && buffer[strlen(buffer) - 2] == '|'))
		return 2;
	else if (buffer[strlen(buffer) - 1] == '|')
		return 1;
	return 0;
}

char *concat_line_continuation(char *buffer, char *new_line, int space_offset)
{
	char *new_buffer;
	new_buffer = calloc(255, sizeof(char));
	buffer[strlen(buffer) + space_offset] = '\0';
	strcpy(new_buffer, buffer);
	strcat(new_buffer, new_line);
	return new_buffer;
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
	}
	return 0;
}

int tokenizer(char *buffer, char **args, char *separator)
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

// Run builtin command or external commands forking and execvp
int standard_execute(char **args)
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
			exit(EXIT_FAILURE);
		}
		{
			waitpid(pid, &status, 0);
			if (status != 0)
				failed_exec = true;
			else
				failed_exec = false;

			return status;
		}
	}
}

// Run commands between ( ) in a special subshell
int subshell_execute(char **sub_line_tokens)
{
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		execute_line(sub_line_tokens);
		exit(EXIT_SUCCESS);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (status != 0)
			failed_exec = true;
		else
			failed_exec = false;

		return status;
	}
}

//TOP
void insert_token(char *buffer, int bf_str, int bf_end, char **line_tokens)
{
	int str_len = &buffer[bf_end] - &buffer[bf_str] + 1;
	*line_tokens = calloc(str_len, sizeof(char));
	strncpy(*line_tokens, &buffer[bf_str], str_len);
}

// TOP
char spec_chs[] = {
	'!',
	'(',
	')',
	';',
	'&',
	'|',
	'\\',
	'\0'};

// TOP
char conc_chs[] = {
	';',
	'|',
	'&',
	'\\'};

// TOP
char *conc_str[] = {
	";",
	"|",
	"||",
	"&&",
	"\\"};

//TOP
bool check_ch(char ch)
{
	for (int i = 0; i < sizeof(spec_chs); i++)
	{
		if (ch == spec_chs[i])
			return true;
	}
	return false;
}

//TOP
bool check_str(char *str)
{
	for (int i = 0; i < sizeof(conc_str) / sizeof(char *); i++)
	{
		if (strcmp(str, conc_str[i]) == 0)
		{
			printf("-> %s\n", conc_str[i]);
			return true;
		}
	}
	return false;
}

// Parsing a line
char **parse_line(char *buffer)
{
	char **line_tokens = calloc(64, sizeof(char *));
	int tk_ind = 0;

	int bf_str = 0;
	int bf_end = 0;

	if (buffer[bf_end] == -1)
		exit(EXIT_SUCCESS);

	while (buffer[bf_end] != '\0')
	{
		if (buffer[bf_str] == ' ')
		{
			bf_str++;
			bf_end++;
		}
		else if (buffer[bf_str] == ';')
		{
			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else if (buffer[bf_str] == '&')
		{
			if (buffer[bf_str + 1] == '&')
				bf_end = bf_str + 1;

			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else if (buffer[bf_str] == '|')
		{
			if (buffer[bf_str + 1] == '|')
				bf_end = bf_str + 1;

			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else if (buffer[bf_str] == '\\')
		{
			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else if (buffer[bf_str] == '(' || buffer[bf_str] == ')')
		{
			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else if (buffer[bf_str] == '!')
		{
			insert_token(buffer, bf_str, bf_end, &line_tokens[tk_ind]);
			bf_end++;
			bf_str = bf_end;
			tk_ind++;
		}
		else
		{
			bf_end++;
			while (!check_ch(buffer[bf_end]))
			{
				//printf("%d ", buffer[bf_end]);
				bf_end++;
			}
			insert_token(buffer, bf_str, bf_end - 1, &line_tokens[tk_ind]);
			bf_str = bf_end;
			tk_ind++;
		}
	}

	return line_tokens;
}
// printf("Buff: %s\n", buffer);
// // Check if there is a concatenator at the end of the string
// int flag_line_cont = check_line_continuation(buffer);
// if (flag_line_cont != 0)
// {
// 	printf("> ");
// 	char *new_line;
// 	read_input(&new_line);
// 	char *new_buffer = concat_line_continuation(buffer, new_line, flag_line_cont);
// 	parse_line(new_buffer, operator_code);
// 	// DELICIOUS SPAGHETTI CODE
// 	return;
// }

// char *brack[2];
// char *next_p;
// int next_operator_code = 0;
// int num_of_op = 4;
// char *operators[num_of_op];
// check_for_brackets(buffer, brack);

// if (get_next_operator(buffer, operators, num_of_op, &next_p) < 0 && brack[0] == (char *)-1)
// {
// 	// Process a single command, no operators or anithing else
// 	char *next_command;
// 	check_code_with_err_then_run(operator_code, buffer);
// }
// else if (brack[0] < next_p)
// {
// 	// This branch is called when the parser has to deal with brackets ( )
// 	char *tmp_buffer = calloc(255, sizeof(char));
// 	strncpy(tmp_buffer, buffer + 1, brack[1] - buffer - 1);
// 	tmp_buffer[strlen(tmp_buffer)] = '\0';
// 	buffer = brack[1] + 1;

// 	// process code in a subshell
// 	subshell_execute(tmp_buffer, operator_code);

// 	// process the remaining commands outside of the subshell
// 	char *connector;
// 	get_next_operator(buffer, operators, num_of_op, &connector);
// 	buffer = buffer + 3;
// 	parse_line(buffer, get_next_operator_code(operators, connector));
// }
// else if (next_p[0] == '|')
// {
// 	char *next_command;
// 	get_next_command(&buffer, &next_command, next_p);
// 	execute_pipe(next_command, operator_code);
// }
// else
// {
// 	// Code that handle a 'canonical' command containing operatos
// 	next_operator_code = get_next_operator_code(operators, next_p);
// 	char *next_command;
// 	get_next_command(&buffer, &next_command, next_p);
// 	check_code_with_err_then_run(operator_code, next_command);
// 	parse_line(buffer, next_operator_code);
// }

void execute_line(char **line_tokens)
{
	int xd = 0;
	while (line_tokens[xd] != 0)
	{
		//printf("Ex this: %s ", line_tokens[xd]);
		xd++;
	}
	int tk_n = 0;
	while (line_tokens[tk_n] != 0)
		tk_n++;

	// First, check for special character at the end of the line
	if (check_str(line_tokens[tk_n - 1]))
	{
		printf("> ");
		char *new_line;
		read_input(&new_line);
		char **new_line_tokens = parse_line(new_line);

		int i = 0;
		int j = tk_n;
		while (new_line_tokens[i] != 0)
		{
			line_tokens[j] = new_line_tokens[i];
			i++;
			j++;
		}

		execute_line(line_tokens);
	}
	else
	{
		int exit_code = 0;
		bool allowed_to_exec = true;

		int line_ind = 0;
		while (line_tokens[line_ind] != 0)
		{
			//printf(" OUT [%s]\n", line_tokens[line_ind]);
			if (strcmp(line_tokens[line_ind], "||") == 0)
			{
				if (exit_code != 0)
					allowed_to_exec = true;
				else
					allowed_to_exec = false;
			}
			else if (strcmp(line_tokens[line_ind], "&&") == 0)
			{
				if (exit_code == 0)
					allowed_to_exec = true;
				else
					allowed_to_exec = false;
			}
			else if (strcmp(line_tokens[line_ind], "(") == 0)
			{
				int i = line_ind + 1;
				int j = 0;
				char **sub_line_tokens = calloc(64, sizeof(char *));
				while (strcmp(line_tokens[i], ")") != 0)
				{
					//printf(" * [%s]\n", line_tokens[i]);
					sub_line_tokens[j] = line_tokens[i];
					i++;
					j++;
				}
				exit_code = subshell_execute(sub_line_tokens);
				line_ind = i;
			}
			else
			{
				if (allowed_to_exec)
				{
					char **p_args = calloc(16, sizeof(char *));
					tokenizer(line_tokens[line_ind], p_args, " ");
					exit_code = standard_execute(p_args);
				}
			}
			line_ind++;
		}
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
	tokenizer(path_var, path_args, ":");

	// Look
	update_current_dir_path();
	update_battery_level();
	update_time();

	// Loop
	while (true)
	{
		terminal();
		// if (feof(stdin))
		// 	exit(EXIT_SUCCESS);

		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == -1)
			continue;

		char **line_tokens = parse_line(buffer);
		execute_line(line_tokens);
	}

	exit(EXIT_SUCCESS);
}

//
// int xd = 0;
// while(sub_line_tokens[xd] != 0){
// 	printf(" - [%s] ", sub_line_tokens[xd]);
// 	xd++;
// }