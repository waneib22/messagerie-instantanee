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
#define MAX_USERS 10

// le nbr max de messages que peut envoyer/recevoir un client
#define MAX_MESSAGES 100

/** le fichier dans lequel les clients
 * et le serveur communiquent
*/
#define SOCK_PATH "./MySock"

char messages[MAX_USERS][MAX_MESSAGES][BUFFER_SIZE];

int message_count[MAX_USERS] = {0};

typedef struct {
    int sock;
    struct sockaddr_un addr;
} client_t;

client_t clients[MAX_USERS];
int num_clients = 0;
pthread_mutex_t mutex_num_client;
pthread_mutex_t mutex_messages;


void save(int user_index, const char* message) {
    pthread_mutex_lock(&mutex_messages);
    int count = message_count[user_index];
    if (count < MAX_MESSAGES) {
        strncpy(messages[user_index][count], message, BUFFER_SIZE);
        message_count[user_index]++;
    }
    pthread_mutex_unlock(&mutex_messages);
}


void get_message(int user_index, char** user_message, int* num_message) {
    pthread_mutex_lock(&mutex_messages);
    * num_message = message_count[user_index];
    for(int i = 0; i < message_count[user_index]; i++) {
        user_message[i] = messages[user_index][i];
    }
    pthread_mutex_unlock(&mutex_messages);
}

/**
 * fonction thread permettant de recevoir 
 * des messages de la part des clients et 
 * d'envoyer des messages au clients
*/
void* handle_client(void* socket_desc) {

    int client_sock = *(int*) socket_desc;
    char buffer[BUFFER_SIZE] = {0};

    while (1)
    {
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) < 0) {
            perror("receiving message from client failed ... \n");
            exit(1);
        }
        buffer[BUFFER_SIZE - 1] = '\0';
        printf("received message from client %d: %s\n", client_sock, buffer);

        int user_index = -1;
        for(int i = 0; i < MAX_USERS; i++) {
            
        }
    }
    

    if (recv(client_sock, buffer, BUFFER_SIZE, 0) < 0) {
        perror("server receiving failed ... \n");
        close(client_sock);
        pthread_exit(NULL);
    }
    printf("Message received : %s\n", buffer);

    pthread_mutex_lock(&mutex_num_client);
    int client_index = num_clients - 1;
    save(client_index, buffer);
    pthread_mutex_unlock(&mutex_num_client);

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

        if (num_clients >= MAX_USERS) {
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
    
    close(server_sock);
    exit(EXIT_SUCCESS);
}