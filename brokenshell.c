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
#include <sys/stat.h>

#define WRITE_END 1
#define READ_END 0

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
----------------------------------------------
|                   Piping                   |
----------------------------------------------
*/

#define WRITE_END 1
#define READ_END 0

int piping_chain(char **commands_list)
{
	int i = 0;
	int read_from_here = STDIN_FILENO;
	pid_t pid;
	int fd[2];

	int status;

	while (commands_list[i] != 0)
	{
		char **cmd = calloc(16, sizeof(char *));
		tokenizer(commands_list[i], cmd, " ");

		pipe(fd);
		pid = fork();

		//CHECK FOR REDIRECTOR TYPE???

		if (pid == 0)
		{
			// Replace stdin fd with my last output fd
			dup2(read_from_here, STDIN_FILENO);

			// Is this teh last pipe chain command? If yes, replace stdout, if no, just use stdout
			if (commands_list[i + 2] != 0)
				dup2(fd[WRITE_END], STDOUT_FILENO);

			//TODO: understand why I have to close ends
			close(fd[READ_END]);
			execvp(cmd[0], cmd);
			fprintf(stderr, "Failed to execvp '%s'\n", cmd[0]);
			exit(EXIT_FAILURE);
		}
		else
		{
			waitpid(pid, &status, 0);
			// after the child has ended, save the fd so the next istruction can read the last output
			read_from_here = fd[READ_END];
			close(fd[WRITE_END]);
		}
		i = i + 2;
	}
	return status;
}

/*
---------------------------------------------------------------
|                   Parse and exec generics                   |
---------------------------------------------------------------
*/

char spec_chs[] = {
	'!',
	'(',
	')',
	';',
	'&',
	'|',
	'\\',
	'\0'};

char conc_chs[] = {
	';',
	'|',
	'&',
	'\\'};

char *conc_str[] = {
	";",
	"|",
	"||",
	"&&"};

// DIsGuStaNg
// TODO refactor
int update_exit_bang(int *bang_flag, int exit_code)
{
	if (*bang_flag == 1)
	{
		*bang_flag = 0;
		if (exit_code == 0)
			return 1;
		else
			return 0;
	}
	return exit_code;
}

bool check_ch(char ch)
{
	for (int i = 0; i < sizeof(spec_chs); i++)
	{
		if (ch == spec_chs[i])
			return true;
	}
	return false;
}

bool check_str(char *str)
{
	for (int i = 0; i < sizeof(conc_str) / sizeof(char *); i++)
	{
		if (strcmp(str, conc_str[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

int point_len(char *arr)
{
	int tmp = 0;
	while (arr[tmp] != '\0')
	{
		tmp++;
	}
	tmp--;
	return tmp;
}

void insert_token(char *buffer, int bf_str, int bf_end, char **line_tokens)
{
	int str_len = &buffer[bf_end] - &buffer[bf_str] + 1;
	*line_tokens = calloc(str_len, sizeof(char));
	strncpy(*line_tokens, &buffer[bf_str], str_len);
}

char **get_pipe_chain(char **line_tokens, int line_ind, int *offset)
{
	char **pipe_chain = calloc(32, sizeof(char *));
	int ind = 0;
	int off = 1;
	while (line_tokens[line_ind + off] != NULL && strcmp(line_tokens[line_ind + off], "|") == 0)
	{
		pipe_chain[ind] = line_tokens[line_ind + off - 1];
		ind++;
		pipe_chain[ind] = line_tokens[line_ind + off];
		ind++;
		off = off + 2;
	}
	pipe_chain[ind] = line_tokens[line_ind + off - 1];
	*offset = ind;
	return pipe_chain;
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

// Parsing a line
char **parse_line(char *buffer)
{
	char **line_tokens = calloc(64, sizeof(char *));
	int tk_ind = 0;

	int bf_str = 0;
	int bf_end = 0;

	// Per passare test
	if (buffer[bf_end] == -1)
		exit(EXIT_SUCCESS);

	// Check if the string ends with '\' to ask for the rest of the input
	if (buffer[point_len(buffer)] == '\\')
	{
		printf("> ");
		char *new_line;
		read_input(&new_line);
		char *new_buffer;
		new_buffer = calloc(64, sizeof(char));
		buffer[strlen(buffer) - 1] = '\0';
		strcpy(new_buffer, buffer);
		strcat(new_buffer, new_line);
		return parse_line(new_buffer);
	}

	// Parsing.
	// It's ugly, but let me change and add operators easily. Will refactor at the end.
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
			// Standard command
			bf_end++;
			while (!check_ch(buffer[bf_end]))
				bf_end++;

			insert_token(buffer, bf_str, bf_end - 1, &line_tokens[tk_ind]);
			bf_str = bf_end;
			tk_ind++;
		}
	}
	return line_tokens;
}

void execute_line(char **line_tokens)
{
	int bang_flag = 0;

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
			if (strcmp(line_tokens[line_ind], "!") == 0)
			{
				bang_flag = 1;
			}
			else if (strcmp(line_tokens[line_ind], ";") == 0)
			{
				allowed_to_exec = true;
			}
			else if (strcmp(line_tokens[line_ind], "||") == 0)
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
					sub_line_tokens[j] = line_tokens[i];
					i++;
					j++;
				}
				exit_code = subshell_execute(sub_line_tokens);
				line_ind = i;
				// TODO refactor
				exit_code = update_exit_bang(&bang_flag, exit_code);
			}
			else
			{
				if (allowed_to_exec)
				{
					// check for pipe/redirect
					if (line_tokens[line_ind + 1] != 0 && strcmp(line_tokens[line_ind + 1], "|") == 0)
					{
						int offset;
						char **pipe_chain = get_pipe_chain(line_tokens, line_ind, &offset);
						exit_code = piping_chain(pipe_chain);
						exit_code = update_exit_bang(&bang_flag, exit_code);
						line_ind = line_ind + offset;
					}
					else
					{
						// Normal execution
						char **p_args = calloc(16, sizeof(char *));
						tokenizer(line_tokens[line_ind], p_args, " ");
						exit_code = standard_execute(p_args);
						// TODO refactor
						exit_code = update_exit_bang(&bang_flag, exit_code);
					}
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

	// Aspect
	update_current_dir_path();
	update_battery_level();
	update_time();

	// Loop
	while (true)
	{
		terminal();
		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == -1)
			continue;

		char **line_tokens = parse_line(buffer);
		execute_line(line_tokens);
	}

	exit(EXIT_SUCCESS);
}