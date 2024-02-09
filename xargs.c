#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


#ifndef NARGS
#define NARGS 4
#endif

int
main(int argc, char *argv[])
{
	if (argc > 2) {
		perror("Cantidad de comandos mayor a 1\n");
		return -1;
	}

	char *paquete[NARGS + 2];

	paquete[0] = argv[1];

	size_t lineas_paquete = 1;

	char *line = NULL;
	size_t len = 0;

	while (getline(&line, &len, stdin) != -1) {
		line[strlen(line) - 1] = '\0';
		paquete[lineas_paquete] = strdup(line);
		lineas_paquete++;

		if (lineas_paquete <= NARGS) {
			continue;
		}

		paquete[lineas_paquete] = NULL;

		pid_t i = fork();

		if (i < 0) {
			perror("Error en fork\n");
			return -1;
		}

		if (i == 0) {
			int e = execvp(argv[1], paquete);
			if (e == -1) {
				perror("Error en execvp\n");
				return -1;
			}
		} else {
			int i_free = 1;
			while (paquete[i_free] != NULL) {
				free(paquete[i_free]);
				paquete[i_free] = NULL;
				i_free++;
			}
			wait(NULL);
		}
		lineas_paquete = 1;
	}

	if (lineas_paquete > 0) {
		paquete[lineas_paquete] = NULL;

		pid_t i = fork();

		if (i < 0) {
			perror("Error en fork\n");
			return -1;
		}

		if (i == 0) {
			int e = execvp(argv[1], paquete);

			if (e == -1) {
				perror("Error en execvp\n");
				return -1;
			}

		} else {
			int i_free = 1;
			while (paquete[i_free] != NULL) {
				free(paquete[i_free]);
				paquete[i_free] = NULL;
				i_free++;
			}
			wait(NULL);
		}
	}
	free(line);
	return 0;
}