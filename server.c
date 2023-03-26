#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>


int main(int argc, char const *argv[])
{

    // declaration des variables 
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    // creation du socket serveur
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Echec creation socket \n");
        exit(1);
    }

    // config du socket
    server_addr = {0};
    server_addr.sun_family = AF_UNIX;
    //strcopy(server_addr.sun_path, "path_to_socket");

    // liaison entre socket et adresse
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Echec liaison socket \n");
        exit(1);
    }


    // ecoute des connexions
    if (listen(server_addr, 10) < 0) {   // backlog = 10
        perror("Echec ecoute \n");
        exit(1);
    }

    // accepter une connexion entrante 
    client_nbr = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addrlen);
    if (client_socket < 0) {
        perror("Echec accept connexion");
        exit(1);
    }

    // pour ce qui est des etablissements de connexion et d'envoi/reception, voir le fichier client.c

    return 0;
}