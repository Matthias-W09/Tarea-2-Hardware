# INTEGRANTES
# Carlos aguirre 20678207-2
# Javier Donetch 20806271-9
# Matias Romero 21319163-2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define NUM_USERS 2

// Función para manejar la comunicación de un usuario
void messagesManager(int read_fd, int write_fd, int user_id) {
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t bytes_read = read(read_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        buffer[bytes_read] = '\0'; 
        // Para detener la ejeccion del programa segun el usuario que lo este utilizando
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Usuario %d recibió la señal de salida. Cerrando...\n", user_id);
            fflush(stdout);
            break;
        }
        // Leer cuando se recibe un mensaje 
        printf("Usuario %d recibió: %s\n", user_id, buffer);
        fflush(stdout);

        // Enviar respuesta
        printf("Usuario %d: ", user_id);
        fflush(stdout);
        fgets(buffer, BUFFER_SIZE, stdin);
        write(write_fd, buffer, strlen(buffer) + 1);
        // Para detener la ejeccion del programa segun el otro usuario
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Usuario %d envió la señal de salida. Cerrando...\n", user_id);
            fflush(stdout);
            break;
        }
    }
    // Cerrar Pipes cuando termina la ejecucion
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

    if (pid1 == 0) { 
        close(pipe1[1]);
        close(pipe2[0]); 
        messagesManager(pipe1[0], pipe2[1], 1);
    }
// Crear segundo proceso hijo (Usuario 2)
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) { 
        close(pipe1[0]); 
        close(pipe2[1]);
        messagesManager(pipe2[0], pipe1[1], 2);
    }

    // Proceso padre
    close(pipe1[0]);
    close(pipe2[1]); 

    //Enviar mensaje inicial al primer proceso hijo
    char *initialMessage = "Iniciando programa de mensajería\n";
    write(pipe1[1], initialMessage, strlen(initialMessage) + 1);

    // Esperar a que los hijos terminen
    close(pipe1[1]);
    close(pipe2[0]);
    wait(NULL);
    wait(NULL);

    return 0;
}
