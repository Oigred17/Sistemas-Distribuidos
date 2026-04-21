#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <ip_servidor> <puerto> <tu_nombre>\n", argv[0]);
        return 1;
    }

    char *ip_server = argv[1];
    int port = atoi(argv[2]);
    char *name = argv[3];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_server, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        return 1;
    }

    printf("\033[1;32m[CONECTADO]\033[0m Bienvenido al Chat Distribuido, %s!\n", name);

    // Enviar nombre como primer mensaje
    send(sock, name, strlen(name), 0);

    fd_set readfds;
    char buffer[BUFFER_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        select(sock + 1, &readfds, NULL, NULL, NULL);

        // Actividad en el teclado (Enviar mensaje)
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                send(sock, buffer, strlen(buffer), 0);
            }
        }

        // Actividad en el socket (Recibir mensaje)
        if (FD_ISSET(sock, &readfds)) {
            int n = recv(sock, buffer, BUFFER_SIZE, 0);
            if (n <= 0) {
                printf("\033[1;31m[ERROR]\033[0m Conexión perdida con el servidor.\n");
                break;
            }
            buffer[n] = '\0';
            printf("%s", buffer);
        }
    }

    close(sock);
    return 0;
}
