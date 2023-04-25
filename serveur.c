#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<pthread.h>

// taille du message
#define BUFFER_SIZE 1024

// le nombre maximum de clients
#define MAX_CLIENTS 10

/**
 * le nombre maximum de messages 
 * que peut recevoir un client
*/
#define MAX_MESSAGES 100

/** le fichier dans lequel les clients
 * et le serveur communiquent
*/
#define SOCK_PATH "./MySock"

typedef struct {
    int sock;
    struct sockaddr_un addr;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t mutex_num_client;

char message_list[MAX_CLIENTS][MAX_MESSAGES][BUFFER_SIZE];
int num_message[MAX_CLIENTS] = {0};
pthread_mutex_t mutex_message_list;


/**
 * fonction ajoutant a chaque reception
 * d'un message du client, le message 
 * dans sa liste
*/
void add_message(int user_index, char* message) {
    pthread_mutex_lock(&mutex_message_list);
    printf("Ajout message %d du client %d \n", num_message[user_index]+1, user_index);
    strcpy(message_list[user_index][num_message[user_index]], message);
    num_message[user_index]++;
    pthread_mutex_unlock(&mutex_message_list);
}


/**
 * fonction retournant tous les 
 * messages envoyés par l'utilisateur
*/
char* get_messages(int user_index) {
    pthread_mutex_lock(&mutex_message_list);
    char* messages = malloc(num_message[user_index] * sizeof(char));
    for(int i = 0; i < num_message[user_index]; i++) {
        messages[i] = message_list[user_index][i];
    }
    return messages;
}


/**
 * fonction thread permettant de recevoir 
 * des messages de la part des clients et 
 * d'envoyer des messages au clients
*/
void* handle_client(void* arg) {

    int client_sock = *(int*) arg;
    char buffer[BUFFER_SIZE] = {0};

    if (recv(client_sock, buffer, BUFFER_SIZE, 0) < 0) {
        perror("server receiving failed ... \n");
        close(client_sock);
        pthread_exit(NULL);
    }
    printf("Message received : %s\n", buffer);

    // recherche d'un client
    int user_index = -1;
    for(int i = 0; i < num_clients; i++) {
        if (clients[i].sock = client_sock) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Client not found in clients list ... \n");
        close(client_sock);
        pthread_exit(NULL);
    }

    /**
     * si client trouve, ajoute son message 
     * dans sa liste
    */
    add_message(user_index, buffer);

    if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
        perror("server sending message failed ... \n");
    } else {
        printf("Message sent : %s\n", buffer);
    }

    if (close(client_sock) != 0) {
        perror("closing client socket failed ... \n");
    }

    pthread_exit(NULL);

}


int main(int argc, char const *argv[])
{
    
    int server_sock, client_sock;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
 
    // congiguration adresse serveur
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);
    
    // configuration adresse client
    memset(&client_addr, 0, sizeof(struct sockaddr_un));

    // initialisation mutex identifiant du client
    pthread_mutex_init(& mutex_num_client, NULL);
    
    // creation du socket serveur
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("creating server socket failed ... \n");
        exit(1);
    }

    /**
     * suppression du chemin d'accès de la communication
     * entre les sockets si il existe
    */
    unlink(SOCK_PATH);
    
    // liaison socket serveur et adresse serveur
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("connecting server socket failed ... \n");
        exit(1);
    }

    // ecoute des connexions
    if (listen(server_sock, 5) != 0) {  
        perror("listening connection failed ... \n");
        exit(1);
    }


    /**
     * dans cette boucle, on génère un thread
     * à chaque nouvelle connexion
    */
    while (1)
    {

        // accepter une connexion
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("accepting client connection failed ...\n");
            continue;
        }

        if (num_clients >= MAX_CLIENTS) {
            printf("Too many clients. Closing connection. \n");
            close(client_sock);
            continue;
        }
        
        /**ajout d'un client à chaque nouvelle
         * connextion en utilisant la structure
         * client_t
        */
        pthread_mutex_lock(&mutex_num_client);
        clients[num_clients].sock = client_sock;
        clients[num_clients].addr = client_addr;
        num_clients++;
        pthread_mutex_unlock(&mutex_num_client);

        /**
         * allocation d'un espace memoire
         * pour le nouveau thread client 
         * généré
        */
        pthread_t thread;
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL) {
            perror("allocating memory for client socket failed ... \n");
            close(client_sock);
            continue;
        }
        *client_sock_ptr = client_sock;

        // creation du thread généré
        if (pthread_create(&thread, NULL, handle_client, (void *)client_sock_ptr) != 0) {
            perror("creating thread for client failed ... \n");
            free(client_sock_ptr);
            close(client_sock);
            continue;
        }

        if (pthread_detach(thread) != 0) {
            perror("detaching thread for client failed");
            free(client_sock_ptr);
            close(client_sock);
            continue;
        }

        printf("created thread for client ... \n");

    }
    
    for (int i = 0; i < num_clients; i++) {
        for (int j = 0; j < num_message[i]; j++) {
            free(message_list[i][j]);
        }
    }

    pthread_mutex_destroy(&mutex_message_list);
    pthread_mutex_destroy(&mutex_num_client);
    close(server_sock);
    exit(EXIT_SUCCESS);
}