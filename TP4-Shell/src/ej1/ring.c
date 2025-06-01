#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n     = atoi(argv[1]);
    int val   = atoi(argv[2]);
    int start = atoi(argv[3]);

    // validación de argumentos 
    if (n < 3) {
        fprintf(stderr, "Error: n debe ser >= 3 (aquí n=%d)\n", n);
        exit(EXIT_FAILURE);
    }
    if (start < 1 || start > n) {
        fprintf(stderr, "Error: s debe estar entre 1 y %d (aquí s=%d)\n", n, start);
        exit(EXIT_FAILURE);
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, val, start);

    // Creamos los pipes del anillo
    int ring[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(ring[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Pipe extra para que el padre inyecte el valor inicial solo al proceso start
    int init_pipe[2];
    if (pipe(init_pipe) == -1) {
        perror("pipe init");
        exit(EXIT_FAILURE);
    }

    // Pipe extra para que el último hijo envíe el resultado de vuelta al padre
    int back[2];
    if (pipe(back) == -1) {
        perror("pipe back");
        exit(EXIT_FAILURE);
    }

    // Fork de los n hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // --- En el hijo i: cerramos todo salvo lo que necesitemos --- //

            // Cerramos extremos de todos los pipes del anillo, excepto aquellos que este hijo usará:
            for (int j = 0; j < n; j++) {
                // cerramos todas las lecturas excepto ring[i][0]
                if (j != i) {
                    close(ring[j][0]);
                }
                // cerramos todas las escrituras excepto ring[(i+1)%n][1]
                if (j != (i + 1) % n) {
                    close(ring[j][1]);
                }
            }

            // Cerramos los extremos de init_pipe que no use cada hijo
            // Si este hijo es el que debe leer el valor inicial (i == start-1),
            //   dejamos abierto init_pipe[0]; cerramos init_pipe[1].
            // Si no, cerramos ambos extremos de init_pipe
            if (i == start - 1) {
                close(init_pipe[1]);
                // init_pipe[0] queda abierto para leer
            } else {
                close(init_pipe[0]);
                close(init_pipe[1]);
            }

            // Cerramos los extremos de back que no use cada hijo
            // Solo el hijo con i == (start-2+n)%n escribirá a back[1]
            // Los demás cierran ambos extremos de back
            if (i == (start - 2 + n) % n) {
                // Este hijo escribe en back[1]
                close(back[0]); 
                // back[1] queda abierto para escribir
            } else {
                close(back[0]);
                close(back[1]);
            }

            // Determinamos de dónde vamos a leer y a dónde vamos a escribir:
            int read_fd, write_fd;
            if (i == start - 1) {
                read_fd = init_pipe[0];
            } else {
                read_fd = ring[i][0];
            }

            if (i == (start - 2 + n) % n) {
                write_fd = back[1];
            } else {
                write_fd = ring[(i + 1) % n][1];
            }

            // leemos, sumamos, escribimos
            int x;
            if (read(read_fd, &x, sizeof(x)) != sizeof(x)) {
                perror("hijo read");
                exit(EXIT_FAILURE);
            }
            x++; 

            if (write(write_fd, &x, sizeof(x)) != sizeof(x)) {
                perror("hijo write");
                exit(EXIT_FAILURE);
            }

            close(read_fd);
            close(write_fd);

            exit(EXIT_SUCCESS);
        }
    }

    // --- En el padre --- //

    // Cerramos extremos de todos los pipes del anillo
    // El padre no lee de anillo ni escribe en él (salvo a través de init_pipe),
    // así que cierra tanto ring[i][0] como ring[i][1] para cada i
    for (int i = 0; i < n; i++) {
        close(ring[i][0]);
        close(ring[i][1]);
    }

    close(init_pipe[0]);

    close(back[1]);

    // pasamos el valor inicial al proceso start
    if (write(init_pipe[1], &val, sizeof(val)) != sizeof(val)) {
        perror("padre write init_pipe");
        exit(EXIT_FAILURE);
    }
    close(init_pipe[1]);

    // Bloqueamos hasta recibir el resultado final desde back[0]
    int final;
    if (read(back[0], &final, sizeof(final)) != sizeof(final)) {
        perror("padre read back");
        exit(EXIT_FAILURE);
    }
    close(back[0]);

    printf("Valor final tras anillo: %d\n", final);

    // Esperamos a todos los hijos para evitar zombies
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;
}
