#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER_KEY 12345

struct message {
    long mtype;
    int client_id;
    char mtext[256];
};

int main() {
    int client_msqid;
    key_t client_key = 1234;
    printf("process key %d \n",client_key);

    // Create client message queue
    if ((client_msqid = msgget(client_key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Connect to the server
    struct message connection_msg;
    connection_msg.mtype = 0; // Use mtype 0 for connection request
    connection_msg.client_id = client_key;

    // Get server message queue
    int server_msqid;
    if ((server_msqid = msgget(SERVER_KEY, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("this is smth %d",server_msqid);
    // Send connection request to the server
    if (msgsnd(server_msqid, &connection_msg, sizeof(struct message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Client: Connected to the server.\n");

    while (1) {
        // Get input from the user
        printf("Enter a request type (1 or 2) and data payload (or 'q' to quit): ");
        char input[256];
        fgets(input, sizeof(input), stdin);
        int input_len = strlen(input);

        // Remove newline character if present
        if (input_len > 0 && input[input_len - 1] == '\n') {
            input[input_len - 1] = '\0';
        }

        // Check if the user wants to quit
        if (strcmp(input, "q") == 0) {
            break;
        }

        // Parse the request type and data payload from the input
        int request_type;
        char data_payload[256];
        sscanf(input, "%d %[^\n]", &request_type, data_payload);

        // Create the message to be sent to the server
        struct message request_msg;
        request_msg.mtype = request_type;
        request_msg.client_id = client_key;
        strncpy(request_msg.mtext, data_payload, sizeof(request_msg.mtext));

        // Send the message to the server
        if (msgsnd(server_msqid, &request_msg, sizeof(struct message), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        // Receive response from the server
        struct message response_msg;
        if (msgrcv(client_msqid, &response_msg, sizeof(struct message), client_key, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Print the response
        printf("Server response: %s\n", response_msg.mtext);
    }

    // Disconnect from the server
    struct message disconnect_msg;
    disconnect_msg.mtype = -1; // Use a special mtype for disconnect request
    disconnect_msg.client_id = client_key;

    // Send disconnect request to the server
    if (msgsnd(server_msqid, &disconnect_msg, sizeof(struct message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    // Remove the client message queue
    if (msgctl(client_msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    printf("Client: Disconnected from the server.\n");

    return 0;
}
