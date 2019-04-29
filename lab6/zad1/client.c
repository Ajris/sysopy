#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "chat.h"

int clientQueueID;
int serverQueueID;
int clientID;
pid_t child_pid;
char* text;
void receiveData(int id, int *type, int *value, int *textSize);

void onQuit();

void handleCtrlC(int signum);

void sigusr_handler(int signum);

void printError();

void send_echo(char *string, size_t size);

void send_list();

void send_to_all(char *string, size_t size);

void send_to_one(char *string, int id, size_t size);

void send_friends(char *string, size_t size);

void send_add_friends(char *string, size_t size);

void send_del_friends(char *string, size_t size);

void send_to_friends(char *string, size_t size);

void handle_input(char *comm, size_t size);

void sendToClient(int id, int type, int value, int textSize);

int main(int argc, char **argv) {
    text = malloc(MAX_MESSAGE_LEN);
    key_t key = ftok(getenv("HOME"), 0);
    if ((serverQueueID = msgget(key, 0)) == -1) printError();
    key_t client_key = ftok(getenv("HOME"), getpid());
    if ((clientQueueID = msgget(client_key, IPC_EXCL | IPC_CREAT | 0666)) == -1) printError();
    atexit(onQuit);
    sendToClient(serverQueueID, INIT, client_key, 0);
    receiveData(clientQueueID, NULL, &clientID, NULL);
    printf("My id: %d\n", clientID);
    size_t max_comm_len = MAX_MESSAGE_LEN + 6;
    char *comm = malloc(MAX_MESSAGE_LEN + 6);
    if ((child_pid = fork()) == -1) printError();
    if (child_pid == 0) {
        int type, value, textSize;
        while (1) {
            receiveData(clientQueueID, &type, &value, &textSize);
            if (type == ECHO) {
                printf("%.*s", textSize, text);
            }
            if (type == STOP) {
                kill(getppid(), SIGUSR1);
                exit(0);
            }
        }
    } else {
        signal(SIGINT, handleCtrlC);
        signal(SIGUSR1, sigusr_handler);
        if (argc > 1) {
            FILE *fd;
            if ((fd = fopen(argv[1], "r")) != NULL) {
                char *input = malloc(5000);
                fread(input, sizeof(char), 5000, fd);
                char *saveptr;
                char *comm = strtok_r(input, "\n", &saveptr);
                while (comm != NULL) {
                    comm[strlen(comm)] = '\n';
                    handle_input(comm, strlen(comm) + 1);
                    comm = strtok_r(NULL, "\n", &saveptr);
                }
            }
        }
        while (1) {
            size_t size = getline(&comm, &max_comm_len, stdin);
            handle_input(comm, size);
        }
    }
}


void sendToClient(int id, int type, int value, int textSize) {
    Message message;
    message.type = type;
    message.value = value;
    message.textSize = textSize;
    if (textSize > MAX_MESSAGE_LEN) {
        fprintf(stderr, "Message too long\n");
        exit(-1);
    }
    if (textSize > 0) {
        memcpy(message.text, text, textSize);
    }
    if (msgsnd(id, &message, MAX_MESSAGE_SIZE, 0) == -1) printError();
}

void receiveData(int id, int *type, int *value, int *textSize) {
    Message message;
    if (msgrcv(id, &message, MAX_MESSAGE_SIZE, 0, 0) == -1) printError();
    if (type != NULL) *type = message.type;
    if (value != NULL) *value = message.value;
    if (textSize != NULL) {
        *textSize = message.textSize;
        if ((*textSize) > 0) {
            memcpy(text, message.text, (*textSize));
        }
    }
}

void onQuit() {
    if (child_pid != 0) {
        msgctl(clientQueueID, IPC_RMID, NULL);
        sendToClient(serverQueueID, STOP, clientID, 0);
        kill(child_pid, SIGKILL);
    }
}

void handleCtrlC(int signum) {
    printf("\nInterrupt signal\n");
    exit(0);
}

void sigusr_handler(int signum) {
    printf("Server stopped\n");
    exit(0);
}

void printError() {
    perror("Error");
    exit(errno);
}

void send_echo(char *string, size_t size) {
    memcpy(text, string + 5, size);
    sendToClient(serverQueueID, ECHO, clientID, size);
}

void send_list() {
    sendToClient(serverQueueID, LIST, 0, 0);
}

void send_to_all(char *string, size_t size) {
    memcpy(text, string + 5, size);
    sendToClient(serverQueueID, TO_ALL, clientID, size);
}

void send_to_one(char *string, int id, size_t size) {
    memcpy(text, string, size);
    sendToClient(serverQueueID, TO_ONE, id, size);
}

void send_friends(char *string, size_t size) {
    memcpy(text, string + 8, size);
    sendToClient(serverQueueID, FRIENDS, clientID, size);
}

void send_add_friends(char *string, size_t size) {
    memcpy(text, string + 4, size);
    sendToClient(serverQueueID, ADD_FRIENDS, clientID, size);
}

void send_del_friends(char *string, size_t size) {
    memcpy(text, string + 4, size);
    sendToClient(serverQueueID, DEL_FRIENDS, clientID, size);
}

void send_to_friends(char *string, size_t size) {
    memcpy(text, string + 9, size);
    sendToClient(serverQueueID, TO_FRIENDS, clientID, size);
}

void handle_input(char *comm, size_t size) {
    if (strncmp(comm, "echo", 4) == 0) {
        send_echo(comm, size - 5);
    } else if (strcmp(comm, "list\n") == 0) {
        send_list();
    } else if (strncmp(comm, "2all", 4) == 0) {
        send_to_all(comm, size - 5);
    } else if (strncmp(comm, "2one", 4) == 0) {
        strtok(comm, " ");
        char *saveptr;
        char *str = strtok_r(comm + 5, " ", &saveptr);
        if (str == NULL) {
            fprintf(stderr, "Wrong number of arguments\n");
            return;
        }
        int id = atoi(str);
        str = strtok_r(NULL, " ", &saveptr);
        size_t size = 0;
        if (str != NULL) size = strlen(str);
        send_to_one(str, id, size);
    } else if (strncmp(comm, "friendsNum", 7) == 0) {
        send_friends(comm, size - 8);
    } else if (strncmp(comm, "add", 3) == 0) {
        send_add_friends(comm, size - 4);
    } else if (strncmp(comm, "del", 3) == 0) {
        send_del_friends(comm, size - 4);
    } else if (strncmp(comm, "2friends", 8) == 0) {
        send_to_friends(comm, size - 9);
    } else if (strcmp(comm, "stop\n") == 0) {
        exit(0);
    } else {
        printf("Wrong command\n");
    }
}
