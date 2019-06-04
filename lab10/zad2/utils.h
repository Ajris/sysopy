#ifndef _UTILS_H
#define _UTILS_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

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
    struct sockaddr *addr;
    socklen_t addr_len;
    uint8_t working;
    uint8_t inactive;
} Client;

int parse_pos_int(char *);

struct timeval curr_time();

long int time_diff(struct timeval, struct timeval);

void print_time(struct timeval t);

void print_curr_time(void);

void show_errno(void);

void die_errno(void);

void die(char *msg);

#endif
