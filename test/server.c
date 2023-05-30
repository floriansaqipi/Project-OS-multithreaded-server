#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_CLIENTS 10 // Maximum number of connected clients

// Define a structure for the message
struct message {
    long mtype; // Message type
    char mtext[100]; // Message text
};

int main() {
    key_t key;
    int msgid;
    struct message msg;

    // Generate a unique key
    key = ftok("server.c", 'A');

    // Create a message queue
    msgid = msgget(key, IPC_CREAT | 0666);

    // List of connected clients
    int clientList[MAX_CLIENTS];
    int numClients = 0;

    // Receive messages indefinitely
    while (1) {
        // Receive the message
        msgrcv(msgid, &msg, sizeof(msg), 1, 0);

        // Add the client to the list
        if (numClients < MAX_CLIENTS) {
            clientList[numClients] = msg.mtype;
            numClients++;
        } else {
            printf("Maximum number of clients reached. Cannot accept more connections.\n");
            continue;
        }

        // Print the received message
        printf("Received message from client %ld: %s\n", msg.mtype, msg.mtext);
    }

    // Remove the message queue
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}