#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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

    // Locate the message queuevfdvf
    msgid = msgget(key, 0666);

    // Send messages to the server
    while (1) {
        printf("Enter a message: ");
        fgets(msg.mtext, sizeof(msg.mtext), stdin);

        // Set the message type
        msg.mtype = 1;

        // Send the message
        msgsnd(msgid, &msg, sizeof(msg), 0);
    }

    return 0;


    
}
