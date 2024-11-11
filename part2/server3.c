#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define ETHSIZE   400
#define PORT 7780
#define BUF_SIZE 5
#define SOLICITUDES_MAXIMA 20
#define CUPO_TOTAL 15
// estrucutura para las solicitudes
typedef struct {
    char name[50];
    long int clv;
    char gpo[200];
} Solicitud;

int main() {
    // arreglo ppara guardar 
    Solicitud solicitudes[SOLICITUDES_MAXIMA];
    int numero_de_solicitudes = 7;
    char request[ETHSIZE];

    printf("\nSe creará el socket...\n");
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);  // Protocolo TCP
    if (socket_fd == -1) {
        perror("\nerror, no se pudo crear el socket.\n");
        exit(-1);
    } else {
        perror("\nSocket creado.\n");
    }

    printf("\nSe asignarán los atributos al socket...\n");
    struct sockaddr_in addr_serv;
    memset(&addr_serv, 0, sizeof addr_serv);
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(PORT);

    int idb = bind(socket_fd, (struct sockaddr *) &addr_serv, sizeof addr_serv);
    if (idb < 0) {
        perror("\nNo se asignaron los atributos.\n");
        close(socket_fd);
        exit(-1);
    } else {
        perror("\nSi se asignaron los atributos.\n");
    }
    if (listen(socket_fd, BUF_SIZE) < 0) {
        perror("\nError al tratar de escuchar.\n");
        close(socket_fd);
        exit(-1);
    }
    printf("\nEsperando conexiones de clientes en el puerto %d...\n", PORT);

    while (1) {
        socklen_t addrLen = sizeof(addr_serv);
        int newSocket_fd = accept(socket_fd, (struct sockaddr *) &addr_serv, &addrLen);
        if (newSocket_fd < 0) {
            perror("\nError al aceptar la conexión del cliente.\n");
            continue;
        }

        int size_recv = read(newSocket_fd, request, ETHSIZE);
        if (size_recv > 0) {
            printf("\nLa petición recibida fue: %s.\n", request);
            // verificamos si aún hay espacio para más solicitudes
            if (numero_de_solicitudes < SOLICITUDES_MAXIMA) {
                // si hay creamos una 
                Solicitud nueva_solicitud;
                sscanf(request, "%[^;];%ld;%s", nueva_solicitud.name, &nueva_solicitud.clv, nueva_solicitud.gpo);
                // la almacenamos en el arreglo
                solicitudes[numero_de_solicitudes++] = nueva_solicitud;

                // imprimimos las solicitudes guardadas
                printf("\nSolicitudes recibidas:\n");
                for (int i = 7; i < numero_de_solicitudes; i++) {
                    printf("Solicitud %d: Alumno=%s, Clave=%ld, Grupo=%s\n", i + 1, solicitudes[i].name, solicitudes[i].clv, solicitudes[i].gpo);
                }

                // respuesta para el cliente
                char respuesta[200];
                sprintf(respuesta, "El cupo total es de : %d, el número de tu solicitud es: %d", CUPO_TOTAL, numero_de_solicitudes);
                send(newSocket_fd, respuesta, strlen(respuesta) + 1, 0);
                printf("\nMensaje enviado al cliente.\n");
            } else {
                printf("\nNo hay espacio para más solicitudes.\n");
            }
        } else {
            perror("\nError al leer la petición del cliente.\n");
        }

        // Cerramos la conexión con el cliente actual
        close(newSocket_fd); 
        printf("\nConexión con el cliente cerrada. Esperando nueva conexión...\n");
    }

    close(socket_fd);
    return 0;
}

