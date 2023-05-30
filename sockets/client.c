#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_MESSAGE_LENGTH 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char message[MAX_MESSAGE_LENGTH];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("address conversion failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // Keep the connection open and allow sending multiple messages
    while (1) {
        printf("Enter your message (or 'exit' to quit): ");
        fgets(message, MAX_MESSAGE_LENGTH, stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character from the message

        if (strcmp(message, "exit") == 0) {
            break; // Exit the loop and close the connection
        }

        // Send message to server
        write(client_socket, message, strlen(message));

        char buffer[MAX_MESSAGE_LENGTH] = {0};
        // Read server response
        read(client_socket, buffer, MAX_MESSAGE_LENGTH);
        printf("Server response: %s\n", buffer);
    }

    printf("Closing the connection.\n");
    close(client_socket);

    return 0;
}
