#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_MESSAGE_SIZE 256

typedef struct {
    long type;
    char text[MAX_MESSAGE_SIZE];
} Message;

int main() {
    // Generate a key for message queues
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int server_queue = msgget(key, 0666);
    if (server_queue == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    int client_queue = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (client_queue == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Send connection request to server's message queue
    Message connection_request;
    connection_request.type = 1;
    sprintf(connection_request.text, "%d", client_queue);
    if (msgsnd(server_queue, &connection_request, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server (client_queue: %d)\n", client_queue);
    printf("Press 'q' to disconnect.\n");

    char message[MAX_MESSAGE_SIZE];

    while (1) {
        // Read a message from the console
        if (fgets(message, MAX_MESSAGE_SIZE, stdin) == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if (message[0] == 'q')
            break;

        Message request;
        request.type = client_queue;
        strcpy(request.text, message);

        // Send request to server's message queue
        if (msgsnd(server_queue, &request, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // Receive response from server's message queue
        Message response;
        if (msgrcv(client_queue, &response, sizeof(Message) - sizeof(long), client_queue, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        // Display the response from the server
        printf("Received response from server: %s", response.text);

        fflush(stdout);  // Flush the output buffer to ensure immediate display
    }

    // Gracefully disconnect from the server
    Message disconnect_request;
    disconnect_request.type = client_queue;
    // Construct disconnect request message...

    // Send disconnect request to server's message queue
    if (msgsnd(server_queue, &disconnect_request, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    msgctl(client_queue, IPC_RMID, NULL);

    return 0;
}
