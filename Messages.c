#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define SOCK_PATH "./MySock"

char **messages; // matrice dynamique pour stocker les messages des utilisateurs
int num_users = 0; // nombre d'utilisateurs connectés

void *connection_handler(void *socket_desc);

int main(int argc, char const *argv[]) {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_un server_sockaddr, client_sockaddr;
    socklen_t client_socklen;
    pthread_t thread_id;
    
    // allocation dynamique de la matrice de messages
    messages = (char **)malloc(MAX_CLIENTS * sizeof(char *));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        messages[i] = (char *)malloc(BUFFER_SIZE * sizeof(char));
        messages[i][0] = '\0';
    }

    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("creating server socket failed ...\n");
        exit(1);
    }

    if (bind(server_sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) != 0) {
        perror("binding server socket failed ...\n");
        exit(1);
    }

    if (listen(server_sock, MAX_CLIENTS) != 0) {
        perror("listening on server socket failed ...\n");
        exit(1);
    }

    printf("server is ready to receive clients ...\n");

    while(1) {
        client_socklen = sizeof(client_sockaddr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_sockaddr, &client_socklen);
        if (client_sock < 0) {
            perror("accepting client connection failed ...\n");
            continue;
        }

        printf("connection accepted from client %d\n", client_sock);

        // création d'un nouveau thread pour gérer la connexion du client
        new_sock = (int*)malloc(1);
        *new_sock = client_sock;
        if (pthread_create(&thread_id, NULL, connection_handler, (void*)new_sock) < 0) {
            perror("could not create thread for client connection ...\n");
            exit(1);
        }
    }

    close(server_sock);
    exit(0);
}

void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];

    while(1) {
        if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
            perror("receiving message from client failed ... \n");
            exit(1);
        }
        buffer[BUFFER_SIZE-1] = '\0';
        printf("received message from client %d: %s\n", sock, buffer);

        // vérification si l'utilisateur est déjà enregistré
        int user_index = -1;
        for (int i = 0; i < num_users; i++) {
    if (strncmp(messages[i], buffer, strlen(buffer)-1) == 0) {
        user_index = i;
        break;
    }
            
void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];

    while(1) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE-1, 0);
        if (bytes_received < 0) {
            perror("Error receiving message from client !\n");
            close(sock);
            free(socket_desc);
            pthread_exit(NULL);
        } else if (bytes_received == 0) {
            printf("Connection closed by client %d\n", sock);
            close(sock);
            free(socket_desc);
            pthread_exit(NULL);
        }

        buffer[bytes_received] = '\0';
        printf("Received message from client %d: %s\n", sock, buffer);

        // Check if the user is in the list
        int user_index = -1;
        for (int i = 0; i < num_users; i++) {
            if (strncmp(messages[i], buffer, strlen(buffer)-1) == 0) {
                user_index = i;
                break;
            }
        }

        // Add user to the list if not the case
        if (user_index == -1) {
            if (num_users >= MAX_CLIENTS) {
                printf("Maximum number of clients reached. Closing connection with client %d\n", sock);
                close(sock);
                free(socket_desc);
                pthread_exit(NULL);
            }
            user_index = num_users;
            num_users++;
            strcpy(messages[user_index], buffer);
            printf("New user registered: %s\n", messages[user_index]);
        }

        // Send received message to all connected users
        for (int i = 0; i < num_users; i++) {
            if (i != user_index) {
                if (send(sockets[i], buffer, strlen(buffer), 0) < 0) {
                    perror("Error sending message to client\n");
                    close(sock);
                    free(socket_desc);
                    pthread_exit(NULL);
                }
            }
        }
    }
}

}
