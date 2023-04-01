#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>

#define SOCK_PATH "/tmp/MySock"


int main(int argc, char const *argv[])
{
    
    // declaration des variables
    int csocket;
    struct sockaddr_un saddr;
    char buffer[1024];


    // config de l'adresse serveur
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, SOCK_PATH);
    //creation socket client
    csocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (csocket < 0) {
        perror("Error creating socket.\n");
        exit(1);
    }

    // liaison socket-adresse
    if (bind(csocket, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Error connecting socket.\n");
        exit(1);
    }

    // etablir une connexion
    if (connect(csocket, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Error connecting.\n");
        exit(1);
    }

    // envoi/reception
    while(1) {
        printf("write your message here.\t");
        fgets(buffer, sizeof(buffer), stdin);
        if (send(csocket, buffer, strlen(buffer), 0) < 0) {
            perror("error sending message.\n");
            exit(1);
        }

        if (recv(csocket, buffer, strlen(buffer), 0) < 0) {
            perror("error receiving message.\n");
            exit(1);
        }
        buffer[1024] = '\0';
        printf("message recu %s\n", buffer);
    }

    close(csocket);
    exit(0);
}