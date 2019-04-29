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
pid_t childPID;
char *text;

Message receiveData(int id);

void onQuit();

void handleCtrlC(int signum);

void handleSIGUSR(int signum);

void printError(char *message);

void sendEcho(char *string, size_t size);

void sendList();

void sendToAll(char *string, size_t size);

void sendToOne(char *string, int id, size_t size);

void sendFriends(char *string, size_t size);

void sendAddFriends(char *string, size_t size);

void sendDelFriends(char *string, size_t size);

void sendToFriends(char *string, size_t size);

void handleInput(char *comm, size_t size);

void sendToClient(int id, int type, int value, int textSize);

int main(int argc, char **argv) {
    text = malloc(MAX_MESSAGE_LEN);
    if ((serverQueueID = msgget(ftok(getenv("HOME"), 0), 0)) == -1)
        printError("Coudlnt get server queue");

    if ((clientQueueID = msgget(ftok(getenv("HOME"), getpid()), IPC_CREAT | 0666)) == -1)
        printError("Coudlnt get client queue");

    atexit(onQuit);

    sendToClient(serverQueueID, INIT, ftok(getenv("HOME"), getpid()), 0);
    Message message = receiveData(clientQueueID);
    clientID = message.value;

    printf("ID: %d\n", clientID);
    if ((childPID = fork()) == 0) {
        while (1) {
            Message received = receiveData(clientQueueID);
            if (received.type == ECHO)
                printf("%s", received.text);
            if (received.type == STOP) {
                kill(getppid(), SIGUSR1);
                exit(0);
            }
        }
    } else {
        signal(SIGINT, handleCtrlC);
        signal(SIGUSR1, handleSIGUSR);
        char *comm = malloc(MAX_MESSAGE_LEN);
        if (argc > 1) {
            FILE *fd;
            if ((fd = fopen(argv[1], "r")) != NULL) {
                char *input = malloc(MAX_FILE_SIZE);
                fread(input, sizeof(char), MAX_FILE_SIZE, fd);
                comm = strtok(input, "\n");
                while (comm != NULL) {
                    comm[strlen(comm)] = '\n';
                    handleInput(comm, strlen(comm) + 1);
                    comm = strtok(NULL, "\n");
                }
            }
        }
        size_t MAX_MESSAGE_LEN_SIZE_T = MAX_MESSAGE_LEN + 6;
        while (1) {
            size_t size = getline(&comm, &MAX_MESSAGE_LEN_SIZE_T, stdin);
            handleInput(comm, size);
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
    if (msgsnd(id, &message, MAX_MESSAGE_SIZE, 0) == -1)
        printError("Coudlnt sent to client");
}

Message receiveData(int id) {
    Message message;
    if (msgrcv(id, &message, MAX_MESSAGE_SIZE, -100, 0) == -1)
        printError("Coudlnt receive");
    text = memcpy(text, message.text, message.textSize);
    return message;
}


void onQuit() {
    if (childPID != 0) {
        msgctl(clientQueueID, IPC_RMID, NULL);
        sendToClient(serverQueueID, STOP, clientID, 0);
        exit(1);
    }
}

void handleCtrlC(int signum) {
    printf("CTRLC PRESSED\n");
    exit(0);
}

void handleSIGUSR(int signum) {
    printf("SERVER STOPPED\n");
    exit(0);
}

void printError(char *message) {
    fprintf(stderr, "%s", message);
    exit(1);
}

void sendEcho(char *string, size_t size) {
    memcpy(text, string + 5, size);
    sendToClient(serverQueueID, ECHO, clientID, size);
}

void sendList() {
    sendToClient(serverQueueID, LIST, 0, 0);
}

void sendToAll(char *string, size_t size) {
    memcpy(text, string + 5, size);
    sendToClient(serverQueueID, TO_ALL, clientID, size);
}

void sendToOne(char *string, int id, size_t size) {
    memcpy(text, string, size);
    sendToClient(serverQueueID, TO_ONE, id, size);
}

void sendFriends(char *string, size_t size) {
    memcpy(text, string + 8, size);
    sendToClient(serverQueueID, FRIENDS, clientID, size);
}

void sendAddFriends(char *string, size_t size) {
    memcpy(text, string + 4, size);
    sendToClient(serverQueueID, ADD_FRIENDS, clientID, size);
}

void sendDelFriends(char *string, size_t size) {
    memcpy(text, string + 4, size);
    sendToClient(serverQueueID, DEL_FRIENDS, clientID, size);
}

void sendToFriends(char *string, size_t size) {
    memcpy(text, string + 9, size);
    sendToClient(serverQueueID, TO_FRIENDS, clientID, size);
}

void handleInput(char *comm, size_t size) {
    if (strncmp(comm, "ECHO", 4) == 0) {
        sendEcho(comm, size - 5);
    } else if (strcmp(comm, "LIST\n") == 0) {
        sendList();
    } else if (strncmp(comm, "2ALL", 4) == 0) {
        sendToAll(comm, size - 5);
    } else if (strncmp(comm, "2ONE", 4) == 0) {
        strtok(comm, " ");
        char *str = strtok(comm + 5, " ");
        if (str == NULL) {
            fprintf(stderr, "Wrong number of arguments\n");
            return;
        }
        int id = atoi(str);
        str = strtok(NULL, " ");
        size_t size = 0;
        if (str != NULL)
            size = strlen(str);
        sendToOne(str, id, size);
    } else if (strncmp(comm, "FRIENDS", 7) == 0) {
        sendFriends(comm, size - 8);
    } else if (strncmp(comm, "ADD", 3) == 0) {
        sendAddFriends(comm, size - 4);
    } else if (strncmp(comm, "DEL", 3) == 0) {
        sendDelFriends(comm, size - 4);
    } else if (strncmp(comm, "2FRIENDS", 8) == 0) {
        sendToFriends(comm, size - 9);
    } else if (strcmp(comm, "STOP\n") == 0) {
        exit(0);
    } else {
        printf("Wrong command\n");
    }
}
