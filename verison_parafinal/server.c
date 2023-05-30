#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_SIZE 256

typedef struct {
    long type;
    char text[MAX_MESSAGE_SIZE];
} Message;

typedef struct {
    int queue_id;
    pthread_t thread;
} Client;

Client connected_clients[MAX_CLIENTS];
pthread_mutex_t client_list_mutex;

void* client_thread(void* arg) {
    Client* client = (Client*)arg;
    int queue_id = client->queue_id;
    Message message;

    while (1) {
        if (msgrcv(queue_id, &message, sizeof(Message) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            break;
        }

        // Process the received message and generate response
        Message response;
        // Process request and generate response...

        response.type = message.type;

        // Send response back to the client
        if (msgsnd(queue_id, &response, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            break;
        }
    }

    // Clean up and disconnect client
    pthread_mutex_lock(&client_list_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connected_clients[i].queue_id == queue_id) {
            msgctl(queue_id, IPC_RMID, NULL);
            connected_clients[i].queue_id = -1;
            break;
        }
    }
    pthread_mutex_unlock(&client_list_mutex);

    printf("Client disconnected (queue_id: %d)\n", queue_id);
    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&client_list_mutex, NULL);

    // Generate a key for message queues
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Initialize message queue for accepting connections
    int server_queue = msgget(key, 0666 | IPC_CREAT);
    if (server_queue == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Server started (queue_id: %d)\n", server_queue);

    // Create a thread for each connected client
    int client_count = 0;
    while (client_count < MAX_CLIENTS) {
        int client_queue = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
        if (client_queue == -1) {
            perror("msgget");
            exit(EXIT_FAILURE);
        }

        connected_clients[client_count].queue_id = client_queue;
        pthread_create(&connected_clients[client_count].thread, NULL, client_thread, (void*)&connected_clients[client_count]);

        // Send connection request to server's message queue
        Message connection_request;
        connection_request.type = 1;
        sprintf(connection_request.text, "%d", client_queue);
        if (msgsnd(server_queue, &connection_request, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        client_count++;
    }

    // Listen for connection requests and add clients to the list
    Message connection_request;
    while (1) {
        if (msgrcv(server_queue, &connection_request, sizeof(Message) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        int client_queue = atoi(connection_request.text);

        pthread_mutex_lock(&client_list_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (connected_clients[i].queue_id == -1) {
                connected_clients[i].queue_id = client_queue;
                pthread_create(&connected_clients[i].thread, NULL, client_thread, (void*)&connected_clients[i]);
                break;
            }
        }
        pthread_mutex_unlock(&client_list_mutex);

        printf("Client connected (queue_id: %d)\n", client_queue);
    }

    pthread_mutex_destroy(&client_list_mutex);
    return 0;
}
