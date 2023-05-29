#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>

#define SERVER_KEY 12345
#define CONNECTION_KEY 54321

struct message {
    long mtype;
    char mtext[256];
};

int main() {
    int client_msqid, server_msqid;
    struct message msg;
    long client_key = getpid();

    // Create client message queue
    if ((client_msqid = msgget(client_key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Get server message queue
    if ((server_msqid = msgget(SERVER_KEY, 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Connect to the server
    struct message connection_msg;
    connection_msg.mtype = CONNECTION_KEY; // Use CONNECTION_KEY as the message type
    if (msgsnd(server_msqid, &connection_msg, sizeof(struct message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Client: Connected to the server.\n");

    while (1) {
        // Get input from the user
        printf("Enter a message to send to the server (or 'q' to quit): ");
        fgets(msg.mtext, sizeof(msg.mtext), stdin);
        int msg_len = strlen(msg.mtext);

        // Remove newline character if present
        if (msg_len > 0 && msg.mtext[msg_len - 1] == '\n') {
            msg.mtext[msg_len - 1] = '\0';
        }

        // Check if the user wants to quit
        if (strcmp(msg.mtext, "q") == 0) {
            break;
        }

        // Send the message to the server
        msg.mtype = client_key; // Use client_key as the message type
        if (msgsnd(client_msqid, &msg, sizeof(struct message), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        // Receive response from the server
        if (msgrcv(client_msqid, &msg, sizeof(struct message), client_key, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Print the response
        printf("Server response: %s\n", msg.mtext);
    }

    // Remove the client message queue
    if (msgctl(client_msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}
