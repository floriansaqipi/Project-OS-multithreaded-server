#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define SERVER_KEY 12345
#define CONNECTION_KEY 54321
#define MAX_CLIENTS 10

struct message {
    long mtype;
    char mtext[256];
};

struct client {
    int msqid;
    // Add any other client-related information you want to track
};

int main() {

        printf("asdfasdfasdfasfasfa");

    int server_msqid, connection_msqid;
    struct client connected_clients[MAX_CLIENTS];
    int num_clients = 0;

    // Create server message queue
    if ((server_msqid = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Create connection message queue
    if ((connection_msqid = msgget(CONNECTION_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Server: Waiting for client connections...\n");


    while (1) {
        struct message connection_msg;

        // Receive connection request from a client
        if (msgrcv(connection_msqid, &connection_msg, sizeof(struct message), CONNECTION_KEY, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Add client to the list of connected clients
        if (num_clients < MAX_CLIENTS) {
            int client_msqid;
            struct client new_client;

            // Get client message queue
            if ((client_msqid = msgget(connection_msg.mtype, 0666)) == -1) {
                perror("msgget");
                exit(1);
            }

            new_client.msqid = client_msqid;
            // Add any other client-related information to the new_client struct

            connected_clients[num_clients++] = new_client;

            printf("Server: Client connected - MSQID: %d\n", client_msqid);
        }
        else {
            printf("Server: Maximum number of clients reached. Connection request rejected.\n");
        }

        for (int i = 0; i < num_clients; i++) {
            struct client current_client = connected_clients[i];
            struct message request_msg, response_msg;

            // Receive request from the current client
            if (msgrcv(current_client.msqid, &request_msg, sizeof(struct message), current_client.msqid, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }

            printf("Server: Received request from client - MSQID: %d, Message: %s\n",
                   current_client.msqid, request_msg.mtext);

            // Process the request (e.g., echo the message)
            printf("Echo: %s", request_msg.mtext);

            // Send response to the current client
            response_msg.mtype = 1; // Use message type 1 for response
            if (msgsnd(current_client.msqid, &response_msg, sizeof(struct message), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }

            printf("Server: Response sent to client - MSQID: %d\n", current_client.msqid);
        }
    }

    return 0;
}
