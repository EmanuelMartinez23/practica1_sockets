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

int main ()
{
   char request[ETHSIZE];
   char datos_para_el_cliente[] = "Rojo";

   printf("\nSe creará el socket...\n");
   int socket_fd = socket(AF_INET, SOCK_STREAM, 0);  // Protocolo TCP
   if (socket_fd == -1) {
      perror("\nerror, no se pudo crear el socket.\n");
      exit(-1);
   }
   else 
      perror("\nSocket creado.\n");

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
   } else
     perror("\nSi se asignaron los atributos.\n");

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
        
         /* Enviar datos al cliente */
         send(newSocket_fd, datos_para_el_cliente, strlen(datos_para_el_cliente) + 1, 0);
         printf("\nMensaje enviado al cliente.\n");
      } else {
         perror("\nError al leer la petición del cliente.\n");
      }

      close(newSocket_fd); // Cerramos la conexión con el cliente actual
      printf("\nConexión con el cliente cerrada. Esperando nueva conexión...\n");
   }

   close(socket_fd);
   return 0;
}

