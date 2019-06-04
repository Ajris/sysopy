#ifndef _UTILS_H
#define _UTILS_H

#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UNIX_PATH_MAX 108
#define MAX_CLIENTS 10

typedef enum SocketMessageType {
    REGISTER,
    UNREGISTER,
    PING,
    PONG,
    OK,
    NAME_TAKEN,
    FULL,
    FAIL,
    WORK,
    WORK_DONE,
} SocketMessageType;

typedef struct SocketMessage {
    uint8_t type;
    uint64_t size;
    uint64_t nameSize;
    uint64_t id;
    void *content;
    char *name;
} SocketMessage;

typedef struct Client {
    int fd;
    char *name;
    uint8_t working;
    uint8_t inactive;
} Client;


void printError(char *message) {
    printf("ERROR: %s\n", message);
    exit(1);
}
#endif
