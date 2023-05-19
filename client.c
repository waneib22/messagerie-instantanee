#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<fcntl.h>
#include<signal.h>

// taille du message
#define BUFFER_SIZE 1024

/** le fichier dans lequel les clients
 * et le serveur communiquent
*/
#define SOCK_PATH "./MySock"

int client_sock;
pthread_cond_t cond_composing_signal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_composing_signal = PTHREAD_MUTEX_INITIALIZER;

void handle_sigint(int signum) {
    pthread_mutex_lock(&mutex_composing_signal);
    printf("Affichage en pause. Entrer votre message:\n");
    pthread_mutex_unlock(&mutex_composing_signal);
}

void* receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];

    while(1) {
        memset(buffer, 0, sizeof(buffer));

        if (recv(client_sock, buffer, sizeof(buffer) -1, 0) < 0) {
            perror("echec reception message ... \n");
            exit(1);
        }

        int clientNumber;
        char* message = NULL;
        sscanf(buffer, "%d %[^\n]", &clientNumber, buffer);

        message = (char*)malloc(strlen(buffer) + 1);
        strcpy(message, buffer);

        pthread_mutex_lock(&mutex_composing_signal);
        printf("Message reçu a partir du client no. %d: %s\n", clientNumber, message);
        pthread_mutex_unlock(&mutex_composing_signal);

        free(message);

        if (strcmp(buffer, "FIN") == 0) {
            break;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{

    struct sockaddr_un server_sockaddr;
    char buffer[BUFFER_SIZE];

    // configuration adresse serveur
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);

    // creation socket client
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("echec creation socket ...\n");
        exit(1);
    }

    // tentative de connexion du client au serveur
    if (connect(client_sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) != 0) {
        perror("echec connexion socket ... \n");
        exit(1);
    }
    
    // installation signal CTRL+C
    signal(SIGINT, handle_sigint);

    pthread_t receive_thread;
    if(pthread_create(&receive_thread, NULL, receive_messages, NULL) < 0) {
        perror("echec creation thread ... \n");
        exit(1);
    }

    printf("Bienvenue dans la messagerie!\n");


    while(1) {
        pthread_mutex_lock(&mutex_composing_signal);
        printf("Entrer votre message (ou 'exit' pour quitter): \n");
        pthread_mutex_unlock(&mutex_composing_signal);

        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("echec envoi message au serveur ... \n");
            exit(1);
        }

        // reception des messages renvoyes par le serveur
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            if (recv(client_sock, buffer, sizeof(buffer) - 1, 0) < 0) {
                perror("echec reception message a partir du serveur ... \n");
                exit(1);
            }

            if (strcmp(buffer, "FIN") == 0) {
                break;
            }
        }

        int clientNumber;

        sscanf(buffer, "%d %[^\n]", &clientNumber, buffer);

        printf("Message recu a partir du client no %d, %s\n", clientNumber, buffer);
    }

    pthread_join(receive_thread, NULL);
    close(client_sock);
    exit(0);

}