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
#include <sys/time.h>

typedef struct Passenger{
    int id;
    int cart;
} Passenger;

typedef struct Cart{
    int id;
    int size;
    int raids;
    Passenger* passIds;
} Cart;

int cartsCount;
int cartSize;
pthread_cond_t* cartsCond;
pthread_mutex_t* cartsMutex;

pthread_mutex_t stationMutex;
pthread_mutex_t passengerMutex;
pthread_mutex_t emptyCartMutex;
pthread_mutex_t fullCartMutex;

pthread_cond_t emptyCondition;
pthread_cond_t fullCondition;

Cart* carts;
int stationCartID;

#endif //LAB9_ZAD1_INFO_H
