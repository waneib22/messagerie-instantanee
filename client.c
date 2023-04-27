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
    
    //Saisie du msg à envoyer : 
    printf("Entrez votre message : ");
    fgets(message, MESSAGE_SIZE, stdin);
    snprintf(message, BUFFER_SIZE, "%s: %s", utilisateur, buffer); 
    // le msg est stocké selon le protocol choisi
    
   /* while(1) {
        printf(">\t");
        fgets(buffer, sizeof(buffer), stdin);
        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("sending message to server failed ... \n");
            exit(1);
        }*/
    
    //Envoi du msg au serveur :
    if (send(sock, message, strlen(message), 0) < 0) {
        printf("Error while sending message.\n");
        return -1;
    }
    printf("Message sent successfully.. \n");        
        
        
    close(client_sock);
    return 0;
}
