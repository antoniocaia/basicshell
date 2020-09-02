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

int check_io_redirect(char **tokens)
{
	int i = 0;
	while (tokens[i] != 0)
	{
		if (check_string_in_list(tokens[i], io_red, 5) != -1)
			return i;
		i++;
	}
	return -1;
}

bool isnumber(char *s)
{
	int i = 0;
	while (s[i] != NULL)
	{
		if (!isdigit(s[i]))
			return false;
		i++;
	}
	return true;
}

void setup_io_red(int tks_ind, char **tokens, int io_ind)
{
	int setter_fd;
	int fd_to_replace;

	switch (io_ind)
	{
	case 0:
		setter_fd = open(tokens[tks_ind + 1], O_RDONLY, 0666);
		fd_to_replace = 0;
		break;
	case 1:
		setter_fd = open(tokens[tks_ind + 1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
		fd_to_replace = 1;
		break;
	case 2:
		setter_fd = open(tokens[tks_ind + 1], O_CREAT | O_APPEND | O_WRONLY, 0666);
		fd_to_replace = 1;
		break;
	case 3:
		setter_fd = open(tokens[tks_ind + 1], O_CREAT | O_RDWR, 0666);
		fd_to_replace = 0;
		break;
	}

	if (isnumber(tokens[tks_ind - 1]))
		fd_to_replace = atoi(tokens[tks_ind]);

	dup2(setter_fd, fd_to_replace);
	close(setter_fd);
}

void setup_io_cpy(int tks_ind, char **tokens, int io_ind)
{
	int setter_fd;
	int fd_to_replace = io_ind;

	if (isnumber(tokens[tks_ind - 1]))
		fd_to_replace = atoi(tokens[tks_ind]);

	if (strcmp(tokens[tks_ind + 1], "-") == 0)
	{
		close(fd_to_replace);
	}
	else
	{
		setter_fd = atoi(tokens[tks_ind + 1]);
		dup2(setter_fd, fd_to_replace);
		close(setter_fd);
	}
}

void execute_command(char **tokens, int *types, int tks_ind)
{
	char **cmd = calloc(16, sizeof(char *));
	cmd[0] = tokens[tks_ind];
	int cmd_ind = 1;
	while (types[tks_ind + 1] == 2)
	{
		cmd[cmd_ind] = tokens[tks_ind];
		cmd_ind;
	}
	execvp(cmd[0], cmd);
}

void execute(char **tokens, int *types)
{
	int io_ind;
	int tks_ind = 0;

	//setup io
	while (tokens[tks_ind] != 0)
	{
		if ((io_ind = check_string_in_list(tokens[tks_ind], io_red, 4) != -1))
			setup_io_red(tks_ind, tokens, io_ind);
		else if ((io_ind = check_string_in_list(tokens[tks_ind], io_cpy, 2) != -1))
			setup_io_cpy(tks_ind, tokens, io_ind);

		tks_ind++;
	}

	tks_ind = 0;
	while (tokens[tks_ind] != 0)
	{
		if (types[tks_ind] == 1)
			execute_command(tokens, types, tks_ind);
		tks_ind++;
	}
}