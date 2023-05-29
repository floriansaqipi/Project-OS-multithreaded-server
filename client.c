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

    // Create client message queue
    if ((client_msqid = msgget(CLIENT_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Get server message queue
    if ((server_msqid = msgget(SERVER_KEY, 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Send request to server
    printf("Client: Enter a message: ");
    fgets(msg.mtext, sizeof(msg.mtext), stdin);
    msg.mtype = 1; // Message type must be greater than 0

    if (msgsnd(server_msqid, &msg, sizeof(struct message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Client: Request sent\n");

    // Receive response from server
    if (msgrcv(client_msqid, &msg, sizeof(struct message), 0, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("Client: Response received - %s\n", msg.mtext);

    // Remove client message queue
    if (msgctl(client_msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}