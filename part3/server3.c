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
#define BUF_SIZE 100
#define NUM_AUTOS 5

// Arreglo de autos: 1 significa disponible, 0 significa ocupado
int autos[NUM_AUTOS] = {0, 0, 1, 0, 1};
int total_viajes = 0;
int ganancia_total = 0;

void manejarCliente(int cliente_fd) {
    char buffer[BUF_SIZE];
    int size_recv = read(cliente_fd, buffer, BUF_SIZE);
    if (size_recv > 0) {
        buffer[size_recv] = '\0';
        printf("Solicitud recibida del cliente:\n\tMensaje: '%s'\n", buffer);

        if (strcmp(buffer, "estado") == 0) {
            // Imprimir detalles sobre el estado actual del servidor
            printf("Petición de estado recibida. Preparando respuesta...\n");
            char respuesta[BUF_SIZE];
            sprintf(respuesta, "Viajes realizados: %d, Ganancia total: %d", total_viajes, ganancia_total);
            printf("\tEnviando al cliente: '%s'\n", respuesta);
            send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);

        } else if (strcmp(buffer, "viaje") == 0) {
            printf("Petición de viaje recibida. Buscando autos disponibles...\n");
            int auto_disponible = -1;
            for (int i = 0; i < NUM_AUTOS; i++) {
                if (autos[i] == 1) {
                    auto_disponible = i;
                    autos[i] = 0;  // Marcar el auto como ocupado
                    printf("\tAuto %d asignado al cliente.\n", i);
                    break;
                }
            }

            if (auto_disponible != -1) {
                int costo = rand() % 50 + 10;  // Costo entre 50 y 100
                total_viajes++;
                ganancia_total += costo;
                char respuesta[BUF_SIZE];
                sprintf(respuesta, "Auto asignado: %d, Costo del viaje: %d", auto_disponible, costo);
                printf("\tEnviando al cliente: 'Auto asignado: %d, Costo del viaje: %d'\n", auto_disponible, costo);
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            } else {
                char respuesta[] = "No hay conductores";
                printf("\tNo hay autos disponibles. Informando al cliente.\n");
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            }

        } else if (strncmp(buffer, "viaje_terminado", 15) == 0) {
            int placas;
            sscanf(buffer, "viaje_terminado %d", &placas);
            printf("Petición de finalización de viaje recibida con placas: %d\n", placas);

            if (placas >= 0 && placas < NUM_AUTOS && autos[placas] == 0) {
                autos[placas] = 1;  // Marcar el auto como disponible
                char respuesta[] = "Auto liberado exitosamente";
                printf("\tEl auto %d ha sido liberado exitosamente.\n", placas);
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            } else {
                char respuesta[] = "Error al liberar el auto";
                printf("\tError al intentar liberar el auto %d. Puede que ya esté disponible o las placas no sean válidas.\n", placas);
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            }
        } else {
            printf("\tSolicitud no reconocida: '%s'\n", buffer);
            char respuesta[] = "Solicitud no reconocida";
            send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
        }
    } else {
        perror("Error al leer la solicitud del cliente");
    }

    // Cerrar la conexión con el cliente
    close(cliente_fd);
    printf("Conexión con el cliente cerrada. Esperando nueva conexión...\n");
}


int main() {
    srand(time(NULL));  // Inicializar la semilla para generar números aleatorios

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr_serv;
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0) {
        perror("Error al asignar los atributos al socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 5) < 0) {
        perror("Error al poner el socket en modo de escucha");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    //printf("Servidor en espera de conexiones en el puerto %d...\n", PORT);
    printf("Somos DidiUAM: %d\n", PORT);

    while (1) {
        int cliente_fd = accept(socket_fd, NULL, NULL);
        if (cliente_fd < 0) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }

        char cliente_ip[INET_ADDRSTRLEN];
        struct sockaddr_in addr_cliente;
        socklen_t addr_len = sizeof(addr_cliente);
        getpeername(cliente_fd, (struct sockaddr *)&addr_cliente, &addr_len);
        inet_ntop(AF_INET, &addr_cliente.sin_addr, cliente_ip, INET_ADDRSTRLEN);
        printf("Conexión recibida de: %s\n", cliente_ip);

        manejarCliente(cliente_fd);
    }

    close(socket_fd);
    return 0;
}
