//
// Created by ajris on 12.05.19.
//


#include "both.h"

key_t getKey() {
    return ftok(getenv("HOME"), PROJECT_ID);
}

void incSem(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = 1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

void decSem(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = -1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

int tryToDecSem(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = IPC_NOWAIT;
    s.sem_num = semaphore;
    s.sem_op = -1;
    if (semop(assemblyLine->semaphoresID, &s, 1) == -1)
        return 1;
    return 0;
}

void putBox(AssemblyLine *assemblyLine, Box box) {
    int sent = 0;
    while (!sent) {
        if (assemblyLine->maxWeight >= box.weight + assemblyLine->currentWeight) {
            box.loadTime = time(NULL);
            assemblyLine->line[assemblyLine->currentBoxInLine++] = box;
            assemblyLine->currentWeight += box.weight;
            assemblyLine->currentBoxes++;
            sent = 1;
            incSem(LINE_SEMAPHORE, assemblyLine);
        }
    }
    printf("PLACED BOX: PID:%d | WEIGHT:%d | TIMEDIFF:%ld | LEFT WEIGHT:%d | LEFT BOXES:%d\n",
           box.workerID, box.weight, box.loadTime, assemblyLine->maxWeight - assemblyLine->currentWeight,
           assemblyLine->maxBoxes - assemblyLine->currentBoxes);
}