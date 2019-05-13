//
// Created by ajris on 12.05.19.
//

#ifndef ZAD1_BOTH_H
#define ZAD1_BOTH_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <sys/time.h>


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
    int semaphoresID;
    int currentBoxInLine;
    int truckEnded;
    Box line[MAX_BOXES_IN_ASSEMBLY_LINE];
} AssemblyLine;

key_t getKey();
void releaseSemaphore(int semaphore, AssemblyLine *assemblyLine);
void takeSemaphore(int semaphore, AssemblyLine *assemblyLine);
int tryToTakeSemaphore(int semaphore, AssemblyLine *assemblyLine);
void putBox(AssemblyLine* assemblyLine, Box box);

#endif //ZAD1_BOTH_H
