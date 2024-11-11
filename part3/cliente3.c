#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7780
#define MAX_BUFFER 256

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Uso: %s <tipo_solicitud> [placas]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int socket_fd;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER];

    // Crear el socket TCP
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Dirección IP no válida");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Preparar y enviar la solicitud
    if (strcmp(argv[1], "estado") == 0) {
        strcpy(buffer, "estado");
    } else if (strcmp(argv[1], "viaje") == 0) {
        strcpy(buffer, "viaje");
    } else if (strcmp(argv[1], "viaje_terminado") == 0 && argc == 3) {
        sprintf(buffer, "viaje_terminado %s", argv[2]);
    } else {
        fprintf(stderr, "Solicitud no válida\n");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    send(socket_fd, buffer, strlen(buffer) + 1, 0);

    // Leer la respuesta del servidor
    memset(buffer, 0, MAX_BUFFER);
    read(socket_fd, buffer, MAX_BUFFER);
    printf("Respuesta del servidor: %s\n", buffer);

    close(socket_fd);
    return 0;
}

