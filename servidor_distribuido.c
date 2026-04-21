#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_CLIENTS 30
#define BUFFER_SIZE 2048

typedef struct {
    int socket;
    char name[50];
} Client;

void broadcast(Client clients[], int max_clients, int sender_fd, int server_peer_fd, char *message) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i].socket > 0 && clients[i].socket != sender_fd) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    // Si tenemos un servidor par y el mensaje no viene de él, se lo enviamos
    if (server_peer_fd > 0 && sender_fd != server_peer_fd) {
        send(server_peer_fd, message, strlen(message), 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <puerto_local> [<ip_remota> <puerto_remoto>]\n", argv[0]);
        return 1;
    }

    int port_local = atoi(argv[1]);
    int master_socket, new_socket;
    Client clients[MAX_CLIENTS];
    int server_peer_socket = -1;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        strcpy(clients[i].name, "Anonimo");
    }

    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_local);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket, 5) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("\033[1;32m[SERVER]\033[0m Servidor escuchando en puerto %d...\n", port_local);

    // Intentar conectar con servidor externo si se provee
    if (argc == 4) {
        char *ip_remota = argv[2];
        int port_remoto = atoi(argv[3]);
        server_peer_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_remoto);
        inet_pton(AF_INET, ip_remota, &server_addr.sin_addr);

        if (connect(server_peer_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            printf("\033[1;36m[PEER]\033[0m Conectado al servidor externo %s:%d\n", ip_remota, port_remoto);
            // Enviar un mensaje de identificación para que el otro sepa que somos un servidor
            char ident[] = "SERVER_NODE";
            send(server_peer_socket, ident, strlen(ident), 0);
        } else {
            printf("\033[1;31m[ERROR]\033[0m No se pudo conectar al servidor externo.\n");
            server_peer_socket = -1;
        }
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        if (server_peer_socket > 0) {
            FD_SET(server_peer_socket, &readfds);
            if (server_peer_socket > max_sd) max_sd = server_peer_socket;
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0) {
                FD_SET(clients[i].socket, &readfds);
                if (clients[i].socket > max_sd) max_sd = clients[i].socket;
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // Nueva conexión entrante
        if (FD_ISSET(master_socket, &readfds)) {
            new_socket = accept(master_socket, NULL, NULL);
            printf("\033[1;34m[INFO]\033[0m Nueva conexión entrante (Socket: %d)\n", new_socket);
            
            int added = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    added = 1;
                    break;
                }
            }
            if (!added) {
                printf("Servidor lleno, rechazando conexión.\n");
                close(new_socket);
            }
        }

        // Mensaje desde el servidor par
        if (server_peer_socket > 0 && FD_ISSET(server_peer_socket, &readfds)) {
            int n = recv(server_peer_socket, buffer, BUFFER_SIZE, 0);
            if (n <= 0) {
                printf("\033[1;31m[PEER]\033[0m Servidor externo desconectado.\n");
                close(server_peer_socket);
                server_peer_socket = -1;
            } else {
                buffer[n] = '\0';
                printf("\033[1;35m[RENAVIO]\033[0m %s", buffer);
                // Difundir a todos los clientes locales
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].socket > 0) send(clients[j].socket, buffer, strlen(buffer), 0);
                }
            }
        }

        // Mensajes de clientes locales
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                int n = recv(sd, buffer, BUFFER_SIZE, 0);
                if (n <= 0) {
                    printf("\033[1;34m[INFO]\033[0m Cliente %s desconectado.\n", clients[i].name);
                    close(sd);
                    clients[i].socket = 0;
                } else {
                    buffer[n] = '\0';
                    
                    // Si el mensaje dice "SERVER_NODE", marcamos este socket como peer si no teníamos uno
                    if (strcmp(buffer, "SERVER_NODE") == 0) {
                        printf("\033[1;36m[PEER]\033[0m Identificado servidor externo entrante.\n");
                        // Lo tratamos diferente (opcionalmente podríamos moverlo a server_peer_socket)
                        // Para simplificar, lo dejamos en la lista de clientes pero sabemos que envía broadcast
                        continue;
                    }

                    // Si es el primer mensaje, podría ser el nombre
                    if (strcmp(clients[i].name, "Anonimo") == 0) {
                        strncpy(clients[i].name, buffer, 49);
                        printf("\033[1;34m[INFO]\033[0m Usuario '%s' se ha unido.\n", clients[i].name);
                        continue;
                    }

                    // Formatear mensaje para broadcast
                    char msg_formatted[BUFFER_SIZE + 100];
                    sprintf(msg_formatted, "[%s]: %s", clients[i].name, buffer);
                    
                    printf("Broadcast local: %s", msg_formatted);
                    broadcast(clients, MAX_CLIENTS, sd, server_peer_socket, msg_formatted);
                }
            }
        }
    }
    return 0;
}
