#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SERVER_KEY 12345
#define CLIENT_KEY 54321

struct message {
    long mtype;
    char mtext[256];
};

int main() {
    int server_msqid, client_msqid;
    struct message msg;

    // Create server message queue
    if ((server_msqid = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1) {
        printf("Failed to accquire message queue UID\n");
        perror("msgget");
        exit(1);
    }

    printf("Server: Waiting for client requests...\n");

    while (1) {
        // Receive client request
        if (msgrcv(server_msqid, &msg, sizeof(struct message), 0, 0) == -1) {
            
            perror("msgrcv");
            exit(1);
        }

        printf("Server: Received request - %s\n", msg.mtext);

        // Process the request (In this example, just echo the message)
        printf("ECHO %s\n",msg.mtext);

        // Get client message queue
        if ((client_msqid = msgget(CLIENT_KEY, 0666)) == -1) {
            perror("msgget");
            exit(1);
        }

        // Send response to client
        msg.mtype = 1; // Message type must be greater than 0
        if (msgsnd(client_msqid, &msg, sizeof(struct message), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        printf("Server: Response sent\n");
    }

    return 0;
}
