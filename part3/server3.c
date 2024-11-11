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

// definimos nuestro arreglo donde 1 es disponible y 0 ocupado
int autos[NUM_AUTOS] = {0, 0, 1, 0, 1};
int total_viajes = 0;
int ganancia_total = 0;

void manejarCliente(int cliente_fd) {
    char buffer[BUF_SIZE];
    // leemos la solicitudl del cliente
    int size_recv = read(cliente_fd, buffer, BUF_SIZE);
    if (size_recv > 0) {
        buffer[size_recv] = '\0';
        // Tipo de solicitud enviada 
        printf("Solicitud recibida del cliente:\n\tTipo de solicitud: '%s'\n", buffer);
        // entra si el tipo de solicitud es la de ver el estado
        if (strcmp(buffer, "estado") == 0) {
            // Imprimir detalles sobre el estado actual del servidor
            // Le retornamos los viajees hasta el momento y la ganacia total
            printf("\tPreparando respuesta.......\n");
            char respuesta[BUF_SIZE];
            sprintf(respuesta,"Viajes realizados hasta el momento: %d, Ganancia total: %d", total_viajes, ganancia_total);
            // debug
            printf("\tInformación enviada al cliente: '%s'\n", respuesta);
            // enviamos la información
            send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);

            // si la solicitud es hacer un viaje entramos
        } else if(strcmp(buffer,"viaje") == 0) {
            printf("\tBuscando autos disponibles......\n");
            // vamos a buscar autos disponibles
            int auto_disponible = -1;
            for (int i = 0; i < NUM_AUTOS; i++) {
                if (autos[i] == 1) {
                    auto_disponible = i;
                    // marcaremos el auto como coupado
                    autos[i] = 0; 
                    // hacemos la asignación al usuario
                    printf("\tAuto número %d asignado al cliente.\n", i);
                    break;
                }
            }
            // si hay autos disponibles entramos 
            if (auto_disponible != -1) {
                // calculamos el costo es random
                int costo = rand() % 50 + 10;
                // incrementamos la variable de los viajes
                total_viajes++;
                // de igual manera la ganacia total
                ganancia_total += costo;
                char respuesta[BUF_SIZE];
                // respuesta para el usuario
                sprintf(respuesta,"Número del auto asignado: %d, costo de su viaje: %d", auto_disponible, costo);
                //debug
                printf("\tInformación para el cliente: 'Número del auto asignado: %d, costo de su viaje: %d'\n", auto_disponible, costo);
                // hacemos la solicitud
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            } else {
                // si no hay lanzamos aviso al usuario
                char respuesta[] = "No hay conductores";
                //debug
                printf("\tNo hay autos disponibles. Informando al cliente.\n");
                // enviamos info al cliente
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            }

            // si la opción es terminado
        } else if (strncmp(buffer, "viaje_terminado", 15) == 0) {
            // almacenamos las placas (el numero)
            int placas;
            // almacenamos el valor que se passo en el parametro de la solicitud
            sscanf(buffer, "viaje_terminado %d", &placas);
            //debug
            printf("\tFinalizando su viaje en el auto: %d\n", placas);
            // marcamos el auto como disponible
            if (placas >= 0 && placas < NUM_AUTOS && autos[placas] == 0) {
                autos[placas] = 1;
                char respuesta[] = "Auto liberado exitosamente";
                //debu
                printf("\tEl auto número %d ha sido liberado exitosamente.\n", placas);
                // info al cliente
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            } else {
                char respuesta[] = "Error al liberar el auto";
                printf("\tError al intentar liberar el auto número %d. Puede que ya esté disponible o las placas no sean válidas.\n", placas);
                send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            }
        } else {
            // cuando la solicitud no es reconocidad
            //debug
            printf("\tSolicitud no reconocida: '%s'\n", buffer);
            char respuesta[] = "No tenemos ese servicio por el momento";
            send(cliente_fd, respuesta, strlen(respuesta) + 1, 0);
            //enviamos al cliente
        }
    } else {
        perror("Error al leer la solicitud del cliente");
    }

    // Cerraramos la conexión con el cliente
    close(cliente_fd);
    printf("\tConexión con el cliente cerrada. Esperando nueva conexión...\n");
    printf("\t.............................................................\n");
}


int main() {
    //semilla para general los numeros aleatorios
    srand(time(NULL));
    // creamos el socket
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
        perror("Error al tratar de escuchar.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    printf("Somos DidiUAM: %d\n", PORT);

    // ietramos para que el servidor reciba continuamennte solicitudes
    while (1) {
        int cliente_fd = accept(socket_fd, NULL, NULL);
        if (cliente_fd < 0) {
            perror("Error al aceptar la conexión del cliente");
            continue;
        }
        // conseguimos el cliente de que IP
        char cliente_ip[INET_ADDRSTRLEN];
        struct sockaddr_in addr_cliente;
        socklen_t addr_len = sizeof(addr_cliente);
        getpeername(cliente_fd, (struct sockaddr *)&addr_cliente, &addr_len);
        inet_ntop(AF_INET, &addr_cliente.sin_addr, cliente_ip, INET_ADDRSTRLEN);
        printf("Conexión recibida de: %s\n", cliente_ip);
        // ejecutamos nuestro metodo que maneja la solicitud de los clientes
        manejarCliente(cliente_fd);
    }

    //cerramos el socket
    close(socket_fd);
    return 0;
}
