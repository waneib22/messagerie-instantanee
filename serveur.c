#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>


int main(int argc, char const *argv[])
{
    
    // create temporary socket file
    char temp[] = "/tmp/MySock";
    int fd = mktemp(temp);
    if (fd == -1) {
        perror("mkdtemp");
        exit(1);
    }
    close(fd);

    // delete any existing element using the same file name
    unlink(temp);


    // declaration des variables 
    int ssocket, csocket;
    struct sockaddr_un saddr, caddr;
    char buffer[1024];

    // config de l'adresse serveur 
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, "/tmp/MySock");
    // config de l'adresse client
    memset(&caddr, 0, sizeof(struct sockaddr_un));
    // creation du socket serveur
    ssocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ssocket < 0) {
        perror("Error creating socket.\n");
        exit(1);
    }

    if (bind(ssocket, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Error connecting socket.\n");
        exit(1);
    }

    if (listen(ssocket, 5) < 0) {  
        perror("Error listening connections \n");
        exit(1);
    }

    while (1)
    {
        int client_len = sizeof(caddr);
        int sservice = accept(ssocket, (struct sockaddr*)&caddr, &client_len);
        if (sservice < 0) {
            perror("Error accepting connection.\n");
            continue;
        }

        // reception 
        if (recv(sservice, buffer, strlen(buffer), 0) < 0) {
            perror("Error receiving data. \n");
            exit(1);
        }
        printf("Message recu : %s\n", buffer);

        // envoi
        strcat(buffer, buffer);
        printf("Message double %s\n", buffer);
        if (send(sservice, buffer, strlen(buffer), 0) < 0) {
            perror("Error sending data. \n");
            exit(1);
        }

        shutdown(sservice, SHUT_RDWR);
        close(sservice);
    }
    
    close(ssocket);
    exit(0);
}