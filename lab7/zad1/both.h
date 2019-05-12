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


#define PROJECT_ID 18
#define MAX_BOXES_IN_ASSEMBLY_LINE 100
#define LINE_SEMAPHORE 0
#define TRUCK_SEMAPHORE 1

typedef struct Truck{
    int maxBoxCount;
    int currentWeight;
    int currentBoxNumber;
} Truck;

typedef struct Package {
    int weight;
    pid_t workerID;
    clock_t loadTime;
} Package;

typedef struct AssemblyLine {
    int maxWeight;
    int maxBoxes;
    int currentWeight;
    int currentBoxes;
    int semaphoresID;
    int currentBoxInLine;
    Package line[MAX_BOXES_IN_ASSEMBLY_LINE];
} AssemblyLine;

key_t getKey();
void incSem(int semaphore, AssemblyLine* assemblyLine);
void decSem(int semaphore, AssemblyLine* assemblyLine);
void tryToDecSem(int semaphore, AssemblyLine* assemblyLine);


#endif //ZAD1_BOTH_H
