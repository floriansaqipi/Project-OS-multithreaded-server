#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
} client_info_t;

void *handle_client(void *arg) {
    client_info_t client_info = *((client_info_t *)arg);
    int client_socket = client_info.socket;
    char buffer[MAX_MESSAGE_LENGTH] = {0};
    char *message = "Server received your message.";

    while (1) {
        // Read client message
        ssize_t bytes_read = read(client_socket, buffer, MAX_MESSAGE_LENGTH);
        if (bytes_read <= 0) {
            break; // Exit the loop if there is an error or the client closes the connection
        }

        printf("Client message: %s\n", buffer);

        // Send response to client
        write(client_socket, message, strlen(message));

        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    }

    printf("Client disconnected. Thread exiting.\n");
    close(client_socket);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t threads[MAX_CLIENTS];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for incoming connections...\n");

    while (1) {
        // Accept a new client connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        client_info_t *client_info = (client_info_t *)malloc(sizeof(client_info_t));
        client_info->socket = client_socket;
        client_info->address = address;

        // Create a new thread for the client
        if (pthread_create(&threads[MAX_CLIENTS], NULL, handle_client, (void *)client_info) < 0) {
            perror("thread creation failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected. Thread created.\n");
    }

    return 0;
}
