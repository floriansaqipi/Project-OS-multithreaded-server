#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>

#define SERVER_KEY 12345
#define MAX_CLIENTS 10

struct message {
    long mtype;
    int client_id;
    char mtext[256];
};

struct client {
    int msqid;
    pthread_t thread;
};

struct client connected_clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* client_thread(void* arg) {
    int client_id = *((int*)arg);
    int client_msqid = connected_clients[client_id].msqid;

    while (1) {
        struct message request_msg, response_msg;

        // Receive request from the client
        if (msgrcv(client_msqid, &request_msg, sizeof(struct message), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("Server: Received request from client %d - Message: %s\n", client_id, request_msg.mtext);

        // Process the request based on the request type
        switch (request_msg.mtype) {
            case 1: // Example request type 1
                // Process the request
                snprintf(response_msg.mtext, sizeof(response_msg.mtext), "Response to request type 1");
                break;

            case 2: // Example request type 2
                // Process the request
                snprintf(response_msg.mtext, sizeof(response_msg.mtext), "Response to request type 2");
                break;

            default:
                snprintf(response_msg.mtext, sizeof(response_msg.mtext), "Unknown request type");
                break;
        }

        response_msg.mtype = connected_clients[client_id].msqid;
        // Send the response to the client
        if (msgsnd(client_msqid, &response_msg, sizeof(struct message), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }
}

int main() {
    int server_msqid;

    // Create server message queue
    if ((server_msqid = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("server message_%d ", server_msqid);

    printf("Server: Waiting for client connections...\n");

    while (1) {
        struct message connection_msg;

        // Receive connection request from a client
        if (msgrcv(server_msqid, &connection_msg, sizeof(struct message), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        int client_msqid = connection_msg.client_id;

        pthread_mutex_lock(&clients_mutex);

        if (num_clients < MAX_CLIENTS) {
            struct client new_client;
            new_client.msqid = client_msqid;

            connected_clients[num_clients] = new_client;
            num_clients++;

            printf("Server: Client connected - ID: %d, MSQID: %d\n", num_clients - 1, client_msqid);

            // Create a new thread for the client
            int client_id = num_clients - 1;
            int *id = malloc(sizeof(int)); 
            *id = client_id;
            pthread_create(&(connected_clients[client_id].thread), NULL, client_thread, id);
        } else {
            printf("Server: Maximum number of clients reached. Connection request rejected.\n");
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}
