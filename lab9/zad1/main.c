//
// Created by ajris on 27.05.19.
//

#include "info.h"

void printError(char *message);

void runPassenger(void* data);

void runCart(void* data);

int main(int argc, char **argv) {
    if(argc != 5)
        printError("Wrong num of arguments");
    int passengersCount = atoi(argv[1]);
    int cartsCount = atoi(argv[2]);
    int cartSize = atoi(argv[3]);
    int raidCount = atoi(argv[4]);

    if(passengersCount <= 0 || cartsCount <= 0 || cartSize <= 0 || raidCount <= 0)
        printError("Error arguemnts");

    pthread_t passThr[passengersCount];
    pthread_t cartThr[cartsCount];
    Passenger passengers[passengersCount];
    Cart carts[cartsCount];

    for(int i = 0; i < passengersCount; i++){
        passengers[i].id = i;
        pthread_create(&passThr[i], NULL, runPassenger, &passengers[i]);
    }

    for(int i = 0; i < cartsCount; i++){
        carts[i].id = i;
        carts[i].size = cartSize;
        carts[i].raids = raidCount;
        pthread_mutex_init(&cartsMutex[i], NULL);
        pthread_cond_init(&cartsCond[i], NULL);
        pthread_create(&cartThr[i], NULL, runCart, &carts[i]);
    }
}

void runPassenger(void* data){
    Passenger* pass = (Passenger*) data;
    while()
}

void runCart(void* cart){

}

void printError(char *message) {
    printf("ERROR: %s\n", message);
    exit(1);
}