#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char **argv)
{
	DIR *dirp;
	struct dirent *file;
	dirp = opendir(".");
	if (dirp == NULL)
	{
		perror("ERROR");
	}

	file = readdir(dirp);

	while (file != NULL)
	{
		if (file->d_type == 4)
			printf("DIRR: %s\n", file->d_name);
		else if (file->d_type == 8)
		{
			if (access(file->d_name, X_OK) == 0)
				printf("EXEC: %s\n", file->d_name);
			else
				printf("FILE: %s\n", file->d_name);
		}
		file = readdir(dirp);
	}
}
