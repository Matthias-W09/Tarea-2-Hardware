#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define NUM_USERS 2

// Función para manejar la comunicación de un usuario
void user_process(int read_fd, int write_fd, int user_id) {
    char buffer[BUFFER_SIZE];
    while (1) {
        // Leer mensaje desde el pipe
        ssize_t bytes_read = read(read_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        buffer[bytes_read] = '\0'; // Asegurar que el buffer esté null-terminated
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Usuario %d recibió la señal de salida. Cerrando...\n", user_id);
            fflush(stdout);
            break;
        }
        printf("Usuario %d recibió: %s\n", user_id, buffer);
        fflush(stdout);

        // Enviar respuesta
        printf("Usuario %d: ", user_id);
        fflush(stdout);
        fgets(buffer, BUFFER_SIZE, stdin);
        write(write_fd, buffer, strlen(buffer) + 1);
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Usuario %d envió la señal de salida. Cerrando...\n", user_id);
            fflush(stdout);
            break;
        }
    }
    close(read_fd);
    close(write_fd);
    exit(0);
}

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid1, pid2;

    // Crear pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Crear primer proceso hijo (Usuario 1)
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) { // Proceso hijo 1 (Usuario 1)
        close(pipe1[1]); // Cerrar extremo de escritura del pipe1
        close(pipe2[0]); // Cerrar extremo de lectura del pipe2
        user_process(pipe1[0], pipe2[1], 1);
    }
// Crear segundo proceso hijo (Usuario 2)
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) { // Proceso hijo 2 (Usuario 2)
        close(pipe1[0]); // Cerrar extremo de lectura del pipe1
        close(pipe2[1]); // Cerrar extremo de escritura del pipe2
        user_process(pipe2[0], pipe1[1], 2);
    }

    // Proceso padre
    close(pipe1[0]); // Cerrar extremos de lectura del pipe1
    close(pipe2[1]); // Cerrar extremos de escritura del pipe2

    //Enviar mensaje inicial al primer proceso hijo
    char *initial_message = "Iniciando programa de mensajería\n";
    write(pipe1[1], initial_message, strlen(initial_message) + 1);

    // Esperar a que los hijos terminen
    close(pipe1[1]);
    close(pipe2[0]);
    wait(NULL);
    wait(NULL);

    return 0;
}