#include "headers.h"

int read_input(char **buffer)
{
	size_t size = 0;
	ssize_t bytes_read = getline(buffer, &size, stdin);

	if (bytes_read == -1)
		exit(EXIT_SUCCESS);
	else if (bytes_read == 1)
		return bytes_read;

	if ((*buffer)[bytes_read - 1] == '\n')
		(*buffer)[bytes_read - 1] = '\0';
	return bytes_read;
}

int main(int argc, char **argv)
{
	while (true)
	{
		printf("> $");
		char *buffer;
		int read_res = read_input(&buffer);
		if (read_res == 1)
			continue;

		tok **line_tok = parse_line(buffer);

		int i = 0;
		while (line_tok[i] != 0)
		{
			printf("[%s]", line_tok[i]->value);
			printf("[%d] ", line_tok[i]->type);
			i++;
		}
		printf("\n---\n");

		execute(line_tok);
	}

	exit(EXIT_SUCCESS);
}