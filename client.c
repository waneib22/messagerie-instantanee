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

    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);

    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("creating client socket failed ...\n");
        exit(1);
    }

    if (connect(client_sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) != 0) {
        perror("connecting to server failed ... \n");
        exit(1);
    }

    while(1) {
        printf(">\t");
        fgets(buffer, sizeof(buffer), stdin);
        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("sending message to server failed ... \n");
            exit(1);
        }

        if (recv(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("receiving message from server failed ... \n");
            exit(1);
        }
        
        buffer[1024] = '\0';
        printf("received message >> %s\n", buffer);
    }

    close(client_sock);
    exit(0);
}