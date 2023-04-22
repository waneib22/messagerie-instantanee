#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<pthread.h>

#define BUFFER_SIZE 1024

#define MAX_CLIENTS 10

#define SOCK_PATH "./MySock"

typedef struct {
    int sock;
    struct sockaddr_un addr;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t mutex_num_client;


void* handle_client(void* arg) {

    int client_sock = *(int*) arg;
    char buffer[BUFFER_SIZE] = {0};

    if (recv(client_sock, buffer, BUFFER_SIZE, 0) < 0) {
        perror("server receiving failed ... \n");
        close(client_sock);
        pthread_exit(NULL);
    }
    printf("Message received : %s\n", buffer);

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
 
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);
    
    memset(&client_addr, 0, sizeof(struct sockaddr_un));

    pthread_mutex_init(& mutex_num_client, NULL);
    
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("creating server socket failed ... \n");
        exit(1);
    }

    unlink(SOCK_PATH);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("connecting server socket failed ... \n");
        exit(1);
    }

    if (listen(server_sock, 5) != 0) {  
        perror("listening connection failed ... \n");
        exit(1);
    }

    while (1)
    {
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
         
        pthread_mutex_lock(&mutex_num_client);
        clients[num_clients].sock = client_sock;
        clients[num_clients].addr = client_addr;
        num_clients++;
        pthread_mutex_unlock(&mutex_num_client);

        pthread_t thread;
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL) {
            perror("allocating memory for client socket failed ... \n");
            close(client_sock);
            continue;
        }
        *client_sock_ptr = client_sock;


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