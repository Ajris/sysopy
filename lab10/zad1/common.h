#ifndef SOCKETS_COMMON_H
#define SOCKETS_COMMON_H

#include <stdint.h>
#include <errno.h>
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

#define MAX_PATH 108
#define CLIENT_MAX 12

typedef enum MessageType {
    REGISTER = 0,
    UNREGISTER = 1,
    SUCCESS = 2,
    FAILSIZE = 3,
    WRONGNAME = 4,
    REQUEST = 5,
    RESULT = 6,
    PING = 7,
    PONG = 8,
    END = 9
} MessageType;

typedef enum ConnectionType {
    LOCAL = 0,
    WEB = 1
} ConnectionType;

typedef struct Message {
    enum MessageType messageType;
    char name[64];
    enum ConnectionType connectionType;
    char value[10240];

} Message;

typedef struct Request{
    char text[10240];
    int ID;
} Request;

typedef struct Client {
    char *name;
    int active_counter;
    enum ConnectionType connectionType;
    struct sockaddr* sockaddr;
    int reserved;
    socklen_t socklen;
} Client;

void printError(char* message) {
    printf("ERROR: %s\n", message);
    exit(1);
}

#endif