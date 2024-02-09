#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

int es_primo_recursiva(int fds_izq[2]);

int
es_primo_recursiva(int fds_izq[2])
{
	close(fds_izq[1]);


	int p;
	int r = read(fds_izq[0], &p, sizeof(p));

	if (r < 0) {
		perror("Error en read\n");
		close(fds_izq[0]);
		return -1;
	}

	if (r == 0) {
		close(fds_izq[0]);
		return 0;
	}

	printf("primo %d\n", p);


	int fds_der[2];
	int pe = pipe(fds_der);

	if (pe < 0) {
		perror("Error al crear el pipe\n");
		close(fds_der[0]);
		close(fds_der[1]);
		close(fds_izq[0]);
		return -1;
	}

	pid_t i = fork();

	if (i < 0) {
		perror("Error en fork\n");
		close(fds_der[0]);
		close(fds_der[1]);
		close(fds_izq[0]);
		return -1;
	}

	if (i == 0) {
		close(fds_izq[0]);
		return es_primo_recursiva(fds_der);

	} else {
		close(fds_der[0]);

		int n;

		r = read(fds_izq[0], &n, sizeof(n));

		if (r < 0) {
			close(fds_izq[0]);
			close(fds_der[1]);
			perror("Error en read\n");
			return -1;
		}

		while (r != 0) {
			if (n % p != 0) {
				int w = write(fds_der[1], &n, sizeof(n));

				if (w < 0) {
					close(fds_izq[0]);
					close(fds_der[1]);
					perror("Error en write\n");
					return -1;
				}
			}
			r = read(fds_izq[0], &n, sizeof(n));

			if (r < 0) {
				close(fds_izq[0]);
				close(fds_der[1]);
				perror("Error en read\n");
				return -1;
			}
		}
		close(fds_izq[0]);
		close(fds_der[1]);
		wait(NULL);
	}
	return 0;
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Cantidad de parametros incorrecta\n");
		return -1;
	}

	int hasta = atoi(argv[1]);

	int fds_der[2];
	int r = pipe(fds_der);

	if (r < 0) {
		close(fds_der[0]);
		close(fds_der[1]);
		perror("Error al crear el pipe\n");
		return -1;
	}

	pid_t i = fork();

	if (i < 0) {
		close(fds_der[0]);
		close(fds_der[1]);
		perror("Error en fork\n");
	}

	if (i == 0) {
		return es_primo_recursiva(fds_der);

	} else {
		close(fds_der[0]);
		int num = 2;
		while (num <= hasta) {
			int w = write(fds_der[1], &num, sizeof(num));

			if (w < 0) {
				perror("Error en write\n");
				close(fds_der[1]);
				return -1;
			}
			num = num + 1;
		}
		close(fds_der[1]);
		wait(NULL);
	}
	return 0;
}