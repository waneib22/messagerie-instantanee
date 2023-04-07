#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>

#define SOCK_PATH "./MySock"


int main(int argc, char const *argv[])
{
    
    int server_sock, client_sock;
    struct sockaddr_un server_sockaddr, client_sockaddr;
    char buffer[1024];
 
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);
    
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket.\n");
        exit(1);
    }

    unlink("./MySock");
    
    if (bind(server_sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        perror("Error connecting socket.\n");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {  
        perror("Error listening connections \n");
        exit(1);
    }

    while (1)
    {
        int client_len = sizeof(client_sockaddr);
        int server_service = accept(server_sock, (struct sockaddr*)&client_sockaddr, &client_len);
        if (server_service < 0) {
            perror("Error accepting connection.\n");
            continue;
        }
         
        if (recv(server_service, buffer, strlen(buffer), 0) < 0) {
            perror("Error receiving data. \n");
            exit(1);
        }
        printf("Message recu : %s\n", buffer);

        strcat(buffer, buffer);
        printf("Message double %s\n", buffer);
        if (send(server_service, buffer, strlen(buffer), 0) < 0) {
            perror("Error sending data. \n");
            exit(1);
        }

        shutdown(server_service, SHUT_RDWR);
        close(server_service);
    }
    
    close(server_sock);
    exit(0);
}