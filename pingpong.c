#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

int
main(void)
{
	printf("Hola, soy PID %d:\n", getpid());

	int fds1[2];
	int r = pipe(fds1);

	if (r < 0) {
		perror("Error en pipe1\n");
		close(fds1[0]);
		close(fds1[1]);
		exit(-1);
	}

	int fds2[2];
	r = pipe(fds2);

	if (r < 0) {
		perror("Error en pipe2\n");
		close(fds1[0]);
		close(fds1[1]);
		close(fds2[0]);
		close(fds2[1]);
		exit(-1);
	}

	printf("  - primer pipe me devuelve: [%d,%d]\n", fds1[0], fds1[1]);
	printf("  - segundo pipe me devuelve: [%d,%d]\n\n", fds2[0], fds2[1]);

	pid_t i = fork();

	if (i < 0) {
		perror("Error en fork\n");
		close(fds1[0]);
		close(fds1[1]);
		close(fds2[0]);
		close(fds2[1]);
		exit(-1);
	}

	if (i == 0) {
		close(fds1[1]);
		close(fds2[0]);
		printf("Donde fork me devuelve 0:\n");
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());

		long numRecibido;
		r = read(fds1[0], &numRecibido, 8);

		close(fds1[0]);

		if (r < 0) {
			close(fds2[1]);
			perror("Error en read\n");
			exit(-1);
		}
		printf("  - recibo valor %ld vía fd=%d\n", numRecibido, fds1[0]);

		r = write(fds2[1], &numRecibido, sizeof(numRecibido));

		close(fds2[1]);

		if (r < 0) {
			perror("Error en write\n");
			exit(-1);
		}
		printf("  - reenvío valor en fd=%d y termino\n\n", fds2[1]);

	} else {
		close(fds1[0]);
		close(fds2[1]);
		printf("Donde fork me devuelve %d:\n", i);
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());

		srandom(time(NULL));
		long numEnviado = random();
		printf("  - random me devuelve: %ld\n", numEnviado);

		r = write(fds1[1], &numEnviado, sizeof(numEnviado));

		close(fds1[1]);

		if (r < 0) {
			close(fds2[0]);
			perror("Error en write\n");
			exit(-1);
		}
		printf("  - envío valor %ld a través de fd=%d\n\n",
		       numEnviado,
		       fds1[1]);

		long numFinal;

		r = read(fds2[0], &numFinal, sizeof(numFinal));

		close(fds2[0]);

		if (r < 0) {
			perror("Error en read\n");
			exit(-1);
		}
		printf("Hola, de nuevo PID %d:\n", getpid());
		printf("  - recibí valor %ld vía fd=%d\n", numFinal, fds2[0]);
		wait(NULL);
	}
	return 0;
}