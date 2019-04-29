//
// Created by ajris on 28.04.19.
//

#include <stdio.h>

#ifndef ZAD1_CHAT_H
#define ZAD1_CHAT_H

enum messageTypes {
    ECHO = 1,
    LIST = 2,
    FRIENDS = 3,
    ADD_FRIENDS = 4,
    DEL_FRIENDS = 5,
    TO_ALL = 6,
    TO_FRIENDS = 7,
    TO_ONE = 8,
    STOP = 9,
    INIT = 10
};

#define MAX_MESSAGE_SIZE sizeof(Message)
#define MAX_MESSAGE_LEN 100
#define MAX_CLIENTS 10

typedef struct Message {
    long type;
    int value;
    size_t textSize;
    char text[MAX_MESSAGE_LEN];
} Message;

#endif //ZAD1_CHAT_H
