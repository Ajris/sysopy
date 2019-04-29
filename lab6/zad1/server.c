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

int serverQueueID;
int lastClientNumber = 0;
int clientsQueueID[MAX_CLIENTS];
int friendsNumber = 0;
int friendsNum[MAX_CLIENTS];
char* text;

void sendToClient(int id, long type, long value, int textSize);

Message receiveData(int id);

void printError(char* mess);

void handleInit(Message message);

void handleEcho(Message message);

void handleToOne(Message message);

void handleList();

void handleToAll(Message message);

void handleFriends(Message message);

void handleAddFriends(Message message);

void handleDelFriends(Message message);

void handleToFriends(Message message);

void handleStop(Message message);

void handleReceived(Message message);

void handleCtrlC(int signum);

void onQuit();

size_t updateTextWithTime(Message message);

int main() {
    text = malloc(MAX_MESSAGE_LEN);
    for(int i = 0; i < MAX_CLIENTS; i++){
        friendsNum[i] = -1;
    }
    signal(SIGINT, handleCtrlC);
    if ((serverQueueID = msgget(ftok(getenv("HOME"), 0), IPC_CREAT | 0666)) == -1)
        printError("Coudlnt get server");
    atexit(onQuit);

    Message message;
    printf("STARTED SERVER: %d\n", serverQueueID);

    while (1) {
        message = receiveData(serverQueueID);
        printf("RECEIVED MESSAGE: TYPE: %ld VALUE: %ld TEXT: %s\n", message.type, message.value, message.text);
        handleReceived(message);
    }
}

size_t updateTextWithTime(Message message){
    time_t now;
    time(&now);
    return sprintf(text, "TEXT: %s TIME: %s ID: %ld", message.text, ctime(&now), message.value);
}

void sendToClient(int id, long type, long value, int textSize) {
    Message message;
    message.type = type;
    message.value = value;
    message.textSize = textSize;
    if (textSize > MAX_MESSAGE_LEN) {
        printError("MESSAGE TOO LONG");
    }
    if (textSize > 0) {
        memcpy(message.text, text, textSize);
    }
    if (msgsnd(id, &message, MAX_MESSAGE_SIZE, 0) == -1)
        printError("Couldnt send");
}

Message receiveData(int id) {
    Message message;
    if (msgrcv(id, &message, MAX_MESSAGE_SIZE, -100, 0) == -1)
        printError("Coudlnt receive");
    text = memcpy(text, message.text, message.textSize);
    return message;
}

void printError(char* message) {
    fprintf(stderr,"%s", message);
    exit(1);
}

void handleInit(Message message) {
    if ((clientsQueueID[lastClientNumber] = msgget(message.value, 0)) == -1)
        printError("Coudlnt get clients queue");
    sendToClient(clientsQueueID[lastClientNumber], INIT, lastClientNumber, 0);
    lastClientNumber++;
}

void handleEcho(Message message) {
    sendToClient(clientsQueueID[message.value], ECHO, 0, updateTextWithTime(message));
}

void handleToOne(Message message) {
    sendToClient(clientsQueueID[message.value], ECHO, 0, updateTextWithTime(message));
}

void handleList() {
    printf("CLIENTS:\n");
    for (int i = 0; i < lastClientNumber; i++) {
        if (clientsQueueID[i] != 0) {
            printf("ID: %d, QUEUEID: %d\n", i, clientsQueueID[i]);
        }
    }
}

void handleToAll(Message message) {
    size_t len = updateTextWithTime(message);
    for (int i = 0; i < lastClientNumber; i++) {
        if (clientsQueueID[i] != 0) {
            sendToClient(clientsQueueID[i], ECHO, 0, len);
        }
    }
}

void handleFriends(Message message) {
    char *tmp = malloc(message.textSize);
    memcpy(tmp, message.text, message.textSize);
    char *str = strtok(tmp, " ");
    friendsNumber = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        friendsNum[i] = -1;
    }
    while (str != NULL) {
        int currentNumExists = 0;
        int num = atoi(str);
        for (int i = 0; i < friendsNumber; i++) {
            if (friendsNum[i] == num) {
                currentNumExists = 1;
            }
        }
        if (currentNumExists) {
            printError("You doubled friends");
        }
        friendsNum[friendsNumber] = num;
        friendsNumber++;
        if(friendsNumber >= MAX_CLIENTS){
            printError("TOO MANY FRIENDS");
        }
        str = strtok(NULL, " ");
    }
}

void handleAddFriends(Message message) {
    char *tmp = malloc(message.textSize);
    memcpy(tmp, message.text, message.textSize);
    char *str = strtok(tmp, " ");
    while (str != NULL) {
        int currentNumExists = 0;
        int num = atoi(str);
        for (int i = 0; i < friendsNumber; i++) {
            if (friendsNum[i] == num) {
                currentNumExists = 1;
            }
        }
        if (currentNumExists) {
            printError("You doubled friends");
        }
        friendsNum[friendsNumber] = num;
        friendsNumber++;

        if(friendsNumber >= MAX_CLIENTS){
            printError("TOO MANY FRIENDS");
        }
        str = strtok(NULL, " ");
    }
}

void handleDelFriends(Message message) {
    char *tmp = malloc(message.textSize);
    memcpy(tmp, message.text, message.textSize);
    char *str = strtok(tmp, " ");
    while (str != NULL) {
        int num = atoi(str);
        for (int i = 0; i < friendsNumber; i++) {
            if (friendsNum[i] == num) {
                for (int j = i; j < friendsNumber - 1; j++) {
                    friendsNum[j] = friendsNum[j + 1];
                }
                friendsNumber--;
            }
        }
        str = strtok(NULL, " ");
    }
}

void handleToFriends(Message message) {
    size_t len = updateTextWithTime(message);
    for (int i = 0; i < friendsNumber; i++) {
        if (clientsQueueID[friendsNum[i]] != 0) {
            sendToClient(clientsQueueID[friendsNum[i]], ECHO, 0, len);
        }
    }
}

void handleStop(Message message) {
    clientsQueueID[message.value] = 0;
}

void handleReceived(Message message) {
    switch (message.type) {
        case INIT:
            handleInit(message);
            break;
        case ECHO:
            handleEcho(message);
            break;
        case TO_ONE:
            handleToOne(message);
            break;
        case LIST:
            handleList();
            break;
        case TO_ALL:
            handleToAll(message);
            break;
        case FRIENDS:
            handleFriends(message);
            break;
        case ADD:
            handleAddFriends(message);
            break;
        case DEL:
            handleDelFriends(message);
            break;
        case TO_FRIENDS:
            handleToFriends(message);
            break;
        case STOP:
            handleStop(message);
            break;
    }
}

void handleCtrlC(int signum) {
    printf("CTRL C PRESSED");
    exit(0);
}

void onQuit() {
    for (int i = 0; i < lastClientNumber; i++) {
        if (clientsQueueID[i] != 0) {
            sendToClient(clientsQueueID[i], STOP, 0, 0);
            receiveData(serverQueueID);
        }
    }
    msgctl(serverQueueID, IPC_RMID, NULL);
}