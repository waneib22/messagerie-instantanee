#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>

#define BUFFER_SIZE 1024

#define SOCK_PATH "./MySock"


int main(int argc, char const *argv[])
{
    
    int client_sock;
    struct sockaddr_un server_sockaddr;
    char buffer[BUFFER_SIZE];

    // configuration adresse serveur
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);

    // creation socket client
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("creating client socket failed ...\n");
        exit(1);
    }

    // tentative de connexion du client au serveur
    if (connect(client_sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) != 0) {
        perror("connecting to server failed ... \n");
        exit(1);
    }

    /**
     * boucle infinie dans laquelle le client 
     * envoie un message au serveur et reçoit
     * un message de la part du serveur 
    */ 
    while(1) {
        printf("Enter your message (or 'exit' to quit): \n");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("sending message to server failed ... \n");
            exit(1);
        }

        memchr(buffer, 0, sizeof(buffer));

        if (recv(client_sock, buffer, sizeof(buffer) - 1, 0) < 0) {
            perror("receiving message failed ... \n");
            exit(1);
        }

        int clientNumber;
        char message[1024];
        sscanf(buffer, "%d %[^/]%s",clientNumber, message);

        printf("received message from client %d, %s\n", clientNumber, message);
    }

    close(client_sock);
    exit(0);

}