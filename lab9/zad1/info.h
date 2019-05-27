//
// Created by ajris on 27.05.19.
//

#ifndef LAB9_ZAD1_INFO_H
#define LAB9_ZAD1_INFO_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>

typedef struct Passenger{
    int id;
} Passenger;

typedef struct Cart{
    int id;
    int size;
    int raids;
} Cart;

pthread_cond_t* cartsCond;
pthread_mutex_t* cartsMutex;
int* cardOrder;

#endif //LAB9_ZAD1_INFO_H
