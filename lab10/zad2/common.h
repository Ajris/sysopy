//
// Created by przjab98 on 31.05.19.
//

#ifndef SOCKETS_COMMON_H
#define SOCKETS_COMMON_H

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#define MAX_TEXT_SIZE 10240
#define MAX_PATH 108
#define CLIENT_MAX 12
#define TYPE_SIZE 1
#define LEN_SIZE 2

#define MAX_MSG_SIZE 10240
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
} MessageType;

typedef enum ConnectionType {
    LOCAL,
    WEB
} ConnectionType;

typedef struct Client {
    int fd;
    char *name;
    int active_counter;
    int reserved;
} Client;

typedef struct Request{
    char text[MAX_MSG_SIZE];
    int ID;
} Request;


void raise_error(char* message){
    fprintf(stderr, "%s :: %s \n", message, strerror(errno));
    exit(1);
}

#endif //SOCKETS_COMMON_H