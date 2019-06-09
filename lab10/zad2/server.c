//
// Created by przjab98 on 31.05.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "common.h"

#define MAX_BUFFER_SIZE 256

int webSocket;
int localSocket;
int epoll;
char *localPath;
int id;

pthread_t ping;
pthread_t command;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Client clients[CLIENT_MAX];
int clientsAmount = 0;

void handleSignal(int);

void init(char *, char *);

void handleMessage(int);

void registerClient(char *, int);

void unregisterClient(char *);

void clean();

void *pingRoutine(void *);

void *handleTerminal(void *);

void handleConnection(int);

void deleteClient(int);

void deleteSocket(int);

size_t getFileSize(const char *fileName) {
    int fd;
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open file for size \n");
        return (size_t) -1;
    }
    struct stat stats;
    fstat(fd, &stats);
    size_t size = (size_t) stats.st_size;
    close(fd);
    return size;
}

size_t readWholeFile(const char *filename, char *buffer) {
    size_t size = getFileSize(filename);
    if (size == -1) {
        return size;
    }
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Unable to open file \n");
        return (size_t) -1;
    }
    size_t read_size;
    if ((read_size = fread(buffer, sizeof(char), size, file)) != size) {
        fprintf(stderr, "Unable to read file\n");
        return (size_t) -1;
    }
    fclose(file);
    return read_size;
}

int currentClient = 0;


int in(void *const a, void *const pbase, size_t totalElems, size_t size, __compar_fn_t cmp) {
    char *basePtr = (char *) pbase;
    if (totalElems > 0) {
        for (int i = 0; i < totalElems; i++) {
            if ((*cmp)(a, (void *) (basePtr + i * size)) == 0) return i;
        }
    }
    return -1;
}

int compareName(char *name, Client *client) {
    return strcmp(name, client->name);
}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 3)
        printError("Provide: port | path\n");

    atexit(clean);
    init(argv[1], argv[2]);

    struct epoll_event event;
    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1)
            printError(" epoll_wait failed\n");
        if (event.data.fd < 0)
            handleConnection(-event.data.fd);
        else
            handleMessage(event.data.fd);
    }
}


void *pingRoutine(void *arg) {
    uint8_t messageType = PING;
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientsAmount; ++i) {
            if (clients[i].activeCounter != 0) {
                printf("num: %d \n", clients[i].activeCounter);
                printf("Client \"%s\" do not respond. Removing from registered clients\n", clients[i].name);
                deleteClient(i--);
            } else {
                if (write(clients[i].fd, &messageType, 1) != 1)
                    printError(" Could not PING client");

                clients[i].activeCounter++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(10);
    }
}

void sendMsg(int type, int len, Request *req, int i) {
    if (write(clients[i].fd, &type, 1) != 1) {
        printError("cannot send");
    }
    if (write(clients[i].fd, &len, 2) != 2) {
        printError("cannot send");
    }
    if (write(clients[i].fd, req, len) != len) {
        printError("cannot send");
    }
}

void *handleTerminal(void *arg) {
    while (1) {
        char buffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, MAX_BUFFER_SIZE);
        printf("Enter command: \n");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        char fileBuffer[MAX_MSG_SIZE];
        memset(fileBuffer, '\0', sizeof(char) * MAX_MSG_SIZE);
        sscanf(buffer, "%s", fileBuffer);
        Request req;
        id++;
        printf("REQUEST ID: %d \n", id);
        printf("%s \n", fileBuffer);
        memset(req.text, 0, sizeof(req.text));
        int status = readWholeFile(fileBuffer, req.text);
        req.ID = id;
        if (strlen(fileBuffer) <= 0) {
            printf("cannot send empty file \n");
            continue;
        }
        if (status < 0) {
            printf("WRONG FILE \n");
            continue;
        }
        int i = currentClient % clientsAmount;
        currentClient++;
        printf("Request sent to %s \n", clients[i].name);
        clients[i].reserved++;
        sendMsg(REQUEST, sizeof(req), &req, i);
    }
}

void handleConnection(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1) printError(" Could not accept new client");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1)
        printError(" Could not add new client to epoll");

}

void handleMessage(int socket) {
    uint8_t messageType;
    uint16_t messageSize;

    if (read(socket, &messageType, TYPE_SIZE) != TYPE_SIZE)
        printError(" Could not read message type\n");
    if (read(socket, &messageSize, LEN_SIZE) != LEN_SIZE)
        printError(" Could not read message size\n");
    char *clientName = malloc(messageSize);

    switch (messageType) {
        case REGISTER: {
            if (read(socket, clientName, messageSize) != messageSize)
                printError(" Could not read register message name");
            registerClient(clientName, socket);
            break;
        }
        case UNREGISTER: {
            if (read(socket, clientName, messageSize) != messageSize)
                printError(" Could not read unregister message name");
            unregisterClient(clientName);
            break;
        }
        case RESULT: {
            printf("SERVER GOT RESULT \n");
            if (read(socket, clientName, messageSize) < 0)
                printError("read res name");
            int size;
            if (read(socket, &size, sizeof(int)) < 0)
                printError("size of res");
            char *result = malloc(size);
            memset(result, 0, size);
            if (read(socket, result, size) < 0)
                printError("result");

            printf("Computations from: %s : %s \n", clientName, result);

            for (int i = 0; i < CLIENT_MAX; i++) {
                if (clients[i].reserved > 0 && strcmp(clientName, clients[i].name) == 0) {
                    clients[i].activeCounter = 0;
                    printf("Client %s is free now \n", clientName);
                }
            }
            free(result);
            break;
        }
        case PONG: {
            if (read(socket, clientName, messageSize) != messageSize)
                printError("Could not read PONG message\n");
            pthread_mutex_lock(&mutex);
            int i = in(clientName, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
            if (i >= 0) clients[i].activeCounter = clients[i].activeCounter == 0 ? 0 : clients[i].activeCounter - 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        default:
            printf("Unknown message type\n");
            break;
    }
    free(clientName);
}

void registerClient(char *clientName, int socket) {
    uint8_t messageType;
    pthread_mutex_lock(&mutex);
    if (clientsAmount == CLIENT_MAX) {
        messageType = FAILSIZE;
        if (write(socket, &messageType, 1) != 1)
            printError(" Could not write FAILSIZE message to client \"%s\"\n");
        deleteSocket(socket);
    } else {
        int exists = in(clientName, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
        if (exists != -1) {
            messageType = WRONGNAME;
            if (write(socket, &messageType, 1) != 1)
                printError(" Could not write WRONGNAME message to client \"%s\"\n");
            deleteSocket(socket);
        } else {
            clients[clientsAmount].fd = socket;
            clients[clientsAmount].name = malloc(strlen(clientName) + 1);
            clients[clientsAmount].activeCounter = 0;
            clients[clientsAmount].reserved = 0;
            strcpy(clients[clientsAmount++].name, clientName);
            messageType = SUCCESS;
            if (write(socket, &messageType, 1) != 1)
                printError(" Could not write SUCCESS message to client \"%s\"\n");
        }
    }
    pthread_mutex_unlock(&mutex);
}

void unregisterClient(char *clientName) {
    pthread_mutex_lock(&mutex);
    int i = in(clientName, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
    if (i >= 0) {
        deleteClient(i);
        printf("Client \"%s\" unregistered\n", clientName);
    }
    pthread_mutex_unlock(&mutex);
}

void deleteClient(int i) {
    deleteSocket(clients[i].fd);
    free(clients[i].name);
    clientsAmount--;
    for (int j = i; j < clientsAmount; ++j)
        clients[j] = clients[j + 1];
}

void deleteSocket(int socket) {
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1)
        printError(" Could not remove client's socket from epoll\n");
    if (shutdown(socket, SHUT_RDWR) == -1) printError(" Could not shutdown client's socket\n");
    if (close(socket) == -1) printError(" Could not close client's socket\n");
}

void handleSignal(int signo) {
    printf("\nSIGINT\n");
    exit(1);
}

void init(char *port, char *path) {
    struct sigaction act;
    act.sa_handler = handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    for (int i = 0; i < CLIENT_MAX; i++) {
        clients[i].reserved = -1;
    }
    uint16_t portNum = (uint16_t) atoi(port);
    localPath = path;

    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
    web_address.sin_port = htons(portNum);

    if ((webSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printError("Could not create web socket");

    int yes = 1;
    if (setsockopt(webSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(webSocket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        printError(" Could not bind web socket\n");

    if (listen(webSocket, 64) == -1)
        printError(" Could not listen to web socket\n");

    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;

    sprintf(local_address.sun_path, "%s", localPath);

    if ((localSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        printError(" Could not create local socket\n");
    if (bind(localSocket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        printError(" Could not bind local socket\n");
    if (listen(localSocket, 64) == -1)
        printError(" Could not listen to local socket\n");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    if ((epoll = epoll_create1(0)) == -1)
        printError(" Could not create epoll\n");
    event.data.fd = -webSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, webSocket, &event) == -1)
        printError(" Could not add Web Socket to epoll\n");
    event.data.fd = -localSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, localSocket, &event) == -1)
        printError(" Could not add Local Socket to epoll\n");

    if (pthread_create(&ping, NULL, pingRoutine, NULL) != 0)
        printError(" Could not create Pinger Thread");
    if (pthread_create(&command, NULL, handleTerminal, NULL) != 0)
        printError(" Could not create Commander Thread");
}

void clean() {
    printf("CLEANUP \n");
    pthread_cancel(ping);
    pthread_cancel(command);
    if (close(webSocket) == -1)
        fprintf(stderr, " Could not close Web Socket\n");
    if (close(localSocket) == -1)
        fprintf(stderr, " Could not close Local Socket\n");
    if (unlink(localPath) == -1)
        fprintf(stderr, " Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, " Could not close epoll\n");
}