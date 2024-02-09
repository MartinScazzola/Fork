#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fcntl.h>

void find(char *palabra,
          char *path,
          DIR *directorio_actual,
          char *(*funcion_str)(const char *, const char *) );

void
find(char *palabra,
     char *path,
     DIR *directorio_actual,
     char *(*funcion_str)(const char *, const char *) )
{
	struct dirent *entrada;

	while ((entrada = readdir(directorio_actual))) {
		if (strcmp(entrada->d_name, ".") == 0 ||
		    strcmp(entrada->d_name, "..") == 0) {
			continue;
		}

		if (entrada->d_type == DT_REG) {
			if (funcion_str(entrada->d_name, palabra)) {
				printf("%s", path);
				printf("%s\n", entrada->d_name);
			}

		} else if (entrada->d_type == DT_DIR) {
			char path_siguiente[PATH_MAX];
			strcpy(path_siguiente, path);
			strcat(path_siguiente, entrada->d_name);

			if (funcion_str(entrada->d_name, palabra)) {
				printf("%s\n", path_siguiente);
			}


			strcat(path_siguiente, "/");

			int fd_ant = dirfd(directorio_actual);

			if (fd_ant < 0) {
				perror("Error en dirfd\n");
				exit(-1);
			}

			int fd_sig = openat(fd_ant, entrada->d_name, O_DIRECTORY);

			if (fd_sig < 0) {
				perror("Error en openat\n");
				exit(-1);
			}

			DIR *directorio_siguiente = fdopendir(fd_sig);

			find(palabra,
			     path_siguiente,
			     directorio_siguiente,
			     funcion_str);
			closedir(directorio_siguiente);
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		perror("Cantidad de parametros incorrecta\n");
		return -1;
	}

	char *(*funcion_str)(const char *, const char *) = &strstr;
	char *palabra = argv[1];

	if (argc == 3) {
		funcion_str = &strcasestr;
		palabra = argv[2];
	}

	DIR *directorio_origen = opendir(".");

	if (directorio_origen == NULL) {
		perror("Error en opendir\n");
		return -1;
	}

	char path[PATH_MAX] = "";

	find(palabra, path, directorio_origen, funcion_str);
	closedir(directorio_origen);
	return 0;
}