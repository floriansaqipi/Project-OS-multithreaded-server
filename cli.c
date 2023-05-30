#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>


#define MAX_MESSAGE_SIZE 256

typedef struct {
    long type;
    char text[MAX_MESSAGE_SIZE];
} Message;

void set_terminal_mode(int enable) {
    static struct termios old_termios, new_termios;
    if (enable) {
        tcgetattr(STDIN_FILENO, &old_termios);
        new_termios = old_termios;
        new_termios.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    }
}

int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) == 1;
}

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

    set_terminal_mode(1);

    while (1) {
        if (kbhit()) {
            char ch = getchar();
            if (ch == 'q') {
                break;
            }

            Message request;
            request.type = client_queue;
            // Set the request text...
            printf("Enter your message: ");
            fgets(request.text, MAX_MESSAGE_SIZE, stdin);

            // Send request to server's message queue
            if (msgsnd(server_queue, &request, sizeof(Message) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        }

        // Check for messages from the server
        Message response;
        if (msgrcv(client_queue, &response, sizeof(Message) - sizeof(long), client_queue, IPC_NOWAIT) != -1) {
            // Process and display the response...
            printf("Received response from server: %s", response.text);
        }
    }

    set_terminal_mode(0);

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
