//
// Created by ajris on 27.05.19.
//

#include "info.h"

void printError(char *message);

void *runPassenger(void *data);

void *runCart(void *data);

struct timeval getCurrentTime();

int main(int argc, char **argv) {
    if (argc != 5)
        printError("Wrong num of arguments");
    int passengersCount = atoi(argv[1]);
    cartsCount = atoi(argv[2]);
    cartSize = atoi(argv[3]);
    int raidCount = atoi(argv[4]);

    if (passengersCount <= 0 || cartsCount <= 0 || cartSize <= 0 || raidCount <= 0)
        printError("Error arguemnts");

    stationCartID = 0;
    pthread_t passThr[passengersCount];
    pthread_t cartThr[cartsCount];
    Passenger passengers[passengersCount];
    carts = malloc(sizeof(Cart) * cartsCount + sizeof(int) * passengersCount);
    cartsMutex = malloc(sizeof(pthread_mutex_t) * cartsCount);
    cartsCond = malloc(sizeof(pthread_cond_t) * cartsCount);

    pthread_mutex_init(&stationMutex, NULL);
    pthread_mutex_init(&emptyCartMutex, NULL);
    pthread_mutex_init(&passengerMutex, NULL);
    pthread_mutex_init(&fullCartMutex, NULL);
    pthread_cond_init(&emptyCondition, NULL);
    pthread_cond_init(&fullCondition, NULL);

    for (int i = 0; i < passengersCount; i++) {
        passengers[i].id = i;
        passengers[i].cart = -1;
    }

    for (int i = 0; i < cartsCount; i++) {
        carts[i].id = i;
        carts[i].size = 0;
        carts[i].raids = raidCount;
        carts[i].passIds = malloc(sizeof(Passenger) * cartSize);
        pthread_mutex_init(&cartsMutex[i], NULL);
        pthread_cond_init(&cartsCond[i], NULL);
    }

    for (int i = 0; i < cartsCount; i++) {
        pthread_create(&cartThr[i], NULL, runCart, &carts[i]);
    }

    for (int i = 0; i < passengersCount; i++) {
        pthread_create(&passThr[i], NULL, runPassenger, &passengers[i]);
    }

    for (int i = 0; i < cartsCount; i++) {
        pthread_join(cartThr[i], NULL);
    }

    for (int i = 0; i < cartsCount; i++) {
        pthread_mutex_destroy(&cartsMutex[i]);
    }
    for(int i = 0; i < cartsCount; i++){
        free(carts[i].passIds);
    }
    free(carts);
    free(cartsMutex);
    free(cartsCond);
}

void *runPassenger(void *data) {
    Passenger *passenger = (Passenger *) data;
    while (1) {
        pthread_mutex_lock(&passengerMutex);

        passenger->cart = stationCartID;
        carts[stationCartID].passIds[carts[stationCartID].size] = *passenger;
        carts[stationCartID].size = carts[stationCartID].size + 1;
        struct timeval curr = getCurrentTime();
        printf("PASSENGER ENTERED || ID: %d PEOPLE IN CART: %d TIME: %ld.%06ld \n", passenger->id,carts[stationCartID].size, curr.tv_sec, curr.tv_usec);

        if(carts[stationCartID].size == cartSize){
            srand(time(NULL));
            curr = getCurrentTime();
            printf("PASSENGER PRESSED START || ID: %d PEOPLE IN CART: %d TIME: %ld.%06ld\n", carts[stationCartID].passIds[rand()%carts->size].id,carts[stationCartID].size, curr.tv_sec, curr.tv_usec);
            pthread_cond_signal(&fullCondition);
            pthread_mutex_unlock(&fullCartMutex);
        } else {
            pthread_mutex_unlock(&passengerMutex);
        }

        pthread_mutex_lock(&cartsMutex[passenger->cart]);
        curr = getCurrentTime();
        carts[stationCartID].size--;
        printf("PASSENGER LEFT || ID: %d PEOPLE IN CART: %d TIME: %ld.%06ld \n", passenger->id, carts[stationCartID].size,curr.tv_sec, curr.tv_usec);
        if(carts[stationCartID].size == 0){
            pthread_cond_signal(&emptyCondition);
            pthread_mutex_unlock(&emptyCartMutex);
        }
        pthread_mutex_unlock(&cartsMutex[passenger->cart]);
        passenger->cart = -1;
    }
}

void *runCart(void *data) {
    Cart *cart = (Cart *) data;
    if (cart->id == 0)
        pthread_mutex_lock(&passengerMutex);

    for (int i = 0; i < cart->raids; i++) {
        pthread_mutex_lock(&stationMutex);
        if (cart->id != stationCartID) {
            pthread_cond_wait(&cartsCond[cart->id], &stationMutex);
        }
        struct timeval curr = getCurrentTime();
        printf("CART COME || ID: %d TIME: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);

        if (i != 0) {
            pthread_mutex_unlock(&cartsMutex[cart->id]);
            pthread_cond_wait(&emptyCondition, &emptyCartMutex);
        }

        pthread_mutex_lock(&cartsMutex[cart->id]);
        pthread_mutex_unlock(&passengerMutex);
        pthread_cond_wait(&fullCondition, &fullCartMutex);

        curr = getCurrentTime();
        printf("CART FULL || ID: %d TIME: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);
        stationCartID = (stationCartID + 1) % cartsCount;

        pthread_cond_signal(&cartsCond[stationCartID]);
        pthread_mutex_unlock(&stationMutex);
    }

    pthread_mutex_lock(&stationMutex);

    if(cart->id != stationCartID) {
        pthread_cond_wait(&cartsCond[cart->id], &stationMutex);
    }

    struct timeval curr = getCurrentTime();
    printf("CART COME || ID: %d TIME: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);

    stationCartID = cart->id;

    pthread_mutex_unlock(&cartsMutex[cart->id]);
    pthread_cond_wait(&emptyCondition,&emptyCartMutex);

    stationCartID = (stationCartID + 1)%cartsCount;

    curr = getCurrentTime();
    printf("CART FINISHED || ID: %d TIME: %ld.%06ld\n", cart->id, curr.tv_sec, curr.tv_usec);

    pthread_cond_signal(&cartsCond[stationCartID]);
    pthread_mutex_unlock(&stationMutex);

    pthread_exit(NULL);
}

struct timeval getCurrentTime() {
    struct timeval curr;
    gettimeofday(&curr, NULL);
    return curr;
}

void printError(char *message) {
    printf("ERROR: %s\n", message);
    exit(1);
}