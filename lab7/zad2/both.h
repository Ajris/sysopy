//
// Created by ajris on 12.05.19.
//

#ifndef ZAD1_BOTH_H
#define ZAD1_BOTH_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#define PROJECT_ID 18
#define MAX_BOXES_IN_ASSEMBLY_LINE 100
#define END_LINE_SEMAPHORE 0
#define TRUCK_SEMAPHORE 1
#define START_LINE_SEMAPHORE 2


typedef struct Truck{
    int maxBoxCount;
    int currentWeight;
    int currentBoxNumber;
} Truck;

typedef struct Box {
    int weight;
    pid_t workerID;
    struct timeval loadTime;
} Box;

typedef struct AssemblyLine {
    int maxWeight;
    int maxBoxes;
    int currentWeight;
    int currentBoxes;
    int currentBoxInLine;
    int truckEnded;
    Box line[MAX_BOXES_IN_ASSEMBLY_LINE];
} AssemblyLine;

sem_t* semaphores[3];

void releaseSemaphore(int semaphore);
void takeSemaphore(int semaphore);
int tryToTakeSemaphore(int semaphore);
void putBox(AssemblyLine* assemblyLine, Box box);

#endif //ZAD1_BOTH_H