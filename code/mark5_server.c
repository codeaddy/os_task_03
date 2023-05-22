#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_QUEUE 5
#define BUFFER_SIZE 1024

pthread_mutex_t barber_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
int clients_in_queue = 0;

int server_fd;

void handle_sigint(int sig) {
    close(server_fd);
    printf("\nServer closed successfully\n");
    exit(0);
}

void *handle_client(void *arg) {
    pthread_mutex_lock(&queue_mutex);
    ++clients_in_queue;
    pthread_mutex_unlock(&queue_mutex);

    pthread_mutex_lock(&barber_mutex);
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    read(new_socket, buffer, BUFFER_SIZE);
    printf("Client: %s\n", buffer);

    printf("Amount of clients in queue: %d\n", clients_in_queue);

    sleep((rand() % 5) + 1);
    char *message = "Your hair has been cut";
    
    pthread_mutex_lock(&queue_mutex);
    --clients_in_queue;
    pthread_mutex_unlock(&queue_mutex);

    send(new_socket, message, strlen(message), 0);

    close(new_socket);
    free(arg);

    pthread_detach(pthread_self());
    pthread_mutex_unlock(&barber_mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_QUEUE) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);

    while (1) {
        printf("Waiting for client...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Client connected\n");

        pthread_t thread_id;
        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;
        pthread_create(&thread_id, NULL, handle_client, new_sock_ptr);
    }

    return 0;
}
