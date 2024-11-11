#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 7780
#define NUM_AUTOS 5
#define MAX_BUFFER 256

typedef struct {
    int num_viajes;
    int ganancia_total;
    int autos_disponibles[NUM_AUTOS];
} Servidor;

void inicializar_servidor(Servidor *servidor) {
    servidor->num_viajes = 0;
    servidor->ganancia_total = 0;
    for (int i = 0; i < NUM_AUTOS; i++) {
        servidor->autos_disponibles[i] = 1; // 1 significa que el auto está disponible
    }
}

int main() {
    Servidor servidor;
    inicializar_servidor(&servidor);
    int socket_fd, newSocket_fd;
    struct sockaddr_in addr_serv, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER];

    // Crear el socket TCP
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(PORT);

    // Enlazar el socket al puerto
    if (bind(socket_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0) {
        perror("Error al enlazar el socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(socket_fd, 5) < 0) {
        perror("Error al escuchar en el socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor activo. Esperando conexiones en el puerto %d...\n", PORT);

    while (1) {
        newSocket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (newSocket_fd < 0) {
            perror("Error al aceptar la conexión");
            continue;
        }

        // Imprimir la IP del cliente
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Conexión recibida de: %s\n", client_ip);

        // Leer la petición del cliente
        memset(buffer, 0, MAX_BUFFER);
        read(newSocket_fd, buffer, MAX_BUFFER);
        printf("Solicitud recibida: %s\n", buffer);

        if (strcmp(buffer, "estado") == 0) {
            // Responder con el estado del servidor
            sprintf(buffer, "Viajes realizados: %d, Ganancia total: %d", servidor.num_viajes, servidor.ganancia_total);
        } else if (strcmp(buffer, "viaje") == 0) {
            int auto_asignado = -1;
            for (int i = 0; i < NUM_AUTOS; i++) {
                if (servidor.autos_disponibles[i] == 1) {
                    auto_asignado = i;
                    servidor.autos_disponibles[i] = 0; // Marcar auto como ocupado
                    break;
                }
            }

            if (auto_asignado != -1) {
                int costo_viaje = (rand() % 100) + 50; // Costo entre 50 y 149
                servidor.num_viajes++;
                servidor.ganancia_total += costo_viaje;
                sprintf(buffer, "Auto asignado: %d, Costo del viaje: %d", auto_asignado, costo_viaje);
            } else {
                strcpy(buffer, "No hay conductores");
            }
        } else if (strncmp(buffer, "viaje_terminado", 15) == 0) {
            int placas;
            sscanf(buffer, "viaje_terminado %d", &placas);
            if (placas >= 0 && placas < NUM_AUTOS && servidor.autos_disponibles[placas] == 0) {
                servidor.autos_disponibles[placas] = 1; // Marcar auto como disponible
                strcpy(buffer, "Viaje finalizado, auto liberado");
            } else {
                strcpy(buffer, "Placas inválidas o auto ya disponible");
            }
        } else {
            strcpy(buffer, "Solicitud no reconocida");
        }

        // Enviar respuesta al cliente
        send(newSocket_fd, buffer, strlen(buffer) + 1, 0);
        close(newSocket_fd);
    }

    close(socket_fd);
    return 0;
}
