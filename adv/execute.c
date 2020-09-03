#include "headers.h"

// DONT CHANGE ORDER!
char *io_red[] = {
	"<", ">", ">>", "<>"};
//	 0	  1	   2	 3

char *io_cpy[] = {
	"<&", ">&"};
//	 0	   1

int check_string_in_list(char *s, char **list, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (strcmp(list[i], s) == 0)
			return i;
	}
	return -1;
}

bool isnumber(char *s)
{
	int i = 0;
	while (s[i] != 0)
	{
		if (!isdigit(s[i]))
			return false;
		i++;
	}
	return true;
}

void setup_io_red(int tks_ind, tok **tokens, int io_ind)
{
	int setter_fd;
	int fd_to_replace;

	switch (io_ind)
	{
	case 0:
		setter_fd = open(tokens[tks_ind + 1]->value, O_RDONLY, 0666);
		fd_to_replace = 0;
		break;
	case 1:
		setter_fd = open(tokens[tks_ind + 1]->value, O_CREAT | O_TRUNC | O_WRONLY, 0666);
		fd_to_replace = 1;
		break;
	case 2:
		setter_fd = open(tokens[tks_ind + 1]->value, O_CREAT | O_APPEND | O_WRONLY, 0666);
		fd_to_replace = 1;
		break;
	case 3:
		setter_fd = open(tokens[tks_ind + 1]->value, O_CREAT | O_RDWR, 0666);
		fd_to_replace = 0;
		break;
	}

	if (tks_ind > 0 && isnumber(tokens[tks_ind - 1]->value))
		fd_to_replace = atoi(tokens[tks_ind]->value);

	dup2(setter_fd, fd_to_replace);
	close(setter_fd);
}

void setup_io_cpy(int tks_ind, tok **tokens, int io_ind)
{
	int setter_fd;
	int fd_to_replace = io_ind;

	if (tks_ind > 0 && isnumber(tokens[tks_ind - 1]->value))
		fd_to_replace = atoi(tokens[tks_ind]->value);

	if (strcmp(tokens[tks_ind + 1]->value, "-") == 0)
	{
		close(fd_to_replace);
	}
	else
	{
		setter_fd = atoi(tokens[tks_ind + 1]->value);
		dup2(setter_fd, fd_to_replace);
		close(setter_fd);
	}
}

int prep_command(tok **tokens, int tks_ind)
{
	int cmd_ind = 0;
	char **cmd = calloc(16, sizeof(char *));
	cmd[cmd_ind] = tokens[tks_ind]->value;
	cmd_ind++;
	tks_ind++;

	while (tokens[tks_ind] != 0 && tokens[tks_ind]->type == 2)
	{
		cmd[cmd_ind] = tokens[tks_ind]->value;
		cmd_ind++;
		tks_ind++;
	}

	return standard_execute(cmd);
}

int standard_execute(char **args)
{
	for (int i = 0; i < builtin_n; i++)
	{
		if (strcmp(args[0], lookup_funct[i]) == 0)
			return builtin_funct[i](args);
	}

	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		execvp(args[0], args);
		exit(EXIT_FAILURE);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (status != 0)
			has_exec_failed = true;
		else
			has_exec_failed = false;

		//printf("ex: %d\n", has_exec_failed);
		return status;
	}
}

bool set_connector(tok *token)
{
	//printf("@@@has_failed: %d \n@@@token value: %s\n", has_exec_failed, token->value);
	if (strcmp(token->value, ";") == 0)
	{
		return true;
	}
	else if (strcmp(token->value, "&&") == 0)
	{
		if (has_exec_failed)
			return false;
		else
			return true;
	}
	else if (strcmp(token->value, "||") == 0)
	{
		if (has_exec_failed)
			return true;
		else
			return false;
	}
}

int shubshell(tok **tokens)
{
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		int e = execute(tokens);
		exit(has_exec_failed);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (status != 0)
			has_exec_failed = true;
		else
			has_exec_failed = false;

		//printf("ex sub: %d\n", has_exec_failed);
		return status;
	}
}

int execute(tok **tokens)
{
	int i = 0;
	bool bang = false;
	bool can_ex = true;
	int io_ind;

	int tks_ind = 0;
	while (tokens[tks_ind] != 0)
	{
		if ((io_ind = check_string_in_list(tokens[tks_ind]->value, io_red, 4) != -1))
			setup_io_red(tks_ind, tokens, io_ind);
		else if ((io_ind = check_string_in_list(tokens[tks_ind]->value, io_cpy, 2) != -1))
			setup_io_cpy(tks_ind, tokens, io_ind);

		tks_ind++;
	}

	// run command
	tks_ind = 0;
	while (tokens[tks_ind] != 0)
	{
		if (tokens[tks_ind]->type == 1)
		{
			if (can_ex)
				prep_command(tokens, tks_ind);
			//printf("final ex: %d\n", has_exec_failed);
			if (bang)
			{
				bang = false;
				has_exec_failed = 1 - has_exec_failed;
				//printf("altered ex: %d\n", has_exec_failed);
			}
		}
		else if (tokens[tks_ind]->type == 3)
		{
			can_ex = set_connector(tokens[tks_ind]);
		}
		else if (strcmp(tokens[tks_ind]->value, "(") == 0)
		{
			tok **sub_tokens = calloc(64, sizeof(tok *));
			int st_ind = 0;
			tks_ind++;
			while ((strcmp(tokens[tks_ind]->value, ")") != 0))
			{
				//printf("tikitak %s\n", tokens[tks_ind]->value);
				sub_tokens[st_ind] = tokens[tks_ind];
				tks_ind++;
				st_ind++;
			}
			shubshell(sub_tokens);
			//printf("final ex: %d\n", has_exec_failed);
			if (bang)
			{
				bang = false;
				has_exec_failed = 1 - has_exec_failed;
				//printf("altered ex: %d\n", has_exec_failed);
			}
			tks_ind;
		}
		else if (strcmp(tokens[tks_ind]->value, "!") == 0)
		{
			bang = true;
		}

		tks_ind++;
	}
}