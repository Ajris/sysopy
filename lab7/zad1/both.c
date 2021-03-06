//
// Created by ajris on 12.05.19.
//


#include "both.h"

key_t getKey() {
    return ftok(getenv("HOME"), PROJECT_ID);
}

void releaseSemaphore(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = 1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

void takeSemaphore(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = -1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

int tryToTakeSemaphore(int semaphore, AssemblyLine *assemblyLine) {
    struct sembuf s;
    s.sem_flg = IPC_NOWAIT;
    s.sem_num = semaphore;
    s.sem_op = -1;
    if (semop(assemblyLine->semaphoresID, &s, 1) >= 0)
        return 1;
    return 0;
}

void putBox(AssemblyLine *assemblyLine, Box box) {
    sleep(rand() % 2);
    int sent = 0;
    while (!sent) {
        if (assemblyLine->truckEnded == 1 && assemblyLine->currentWeight == 0)
            exit(0);
        takeSemaphore(START_LINE_SEMAPHORE, assemblyLine);
        if (assemblyLine->maxWeight >= box.weight + assemblyLine->currentWeight &&
            assemblyLine->currentBoxes + 1 <= assemblyLine->maxBoxes) {
            if(tryToTakeSemaphore(TRUCK_SEMAPHORE, assemblyLine) == 1){
                gettimeofday(&box.loadTime, NULL);
                assemblyLine->line[assemblyLine->currentBoxInLine++] = box;
                assemblyLine->currentBoxInLine %= MAX_BOXES_IN_ASSEMBLY_LINE;
                assemblyLine->currentWeight += box.weight;
                assemblyLine->currentBoxes++;
                sent = 1;
                releaseSemaphore(END_LINE_SEMAPHORE, assemblyLine);
            } else {
                printf("TRUCK WILL BE FULL\n");
            }
        }
        releaseSemaphore(START_LINE_SEMAPHORE, assemblyLine);
    }

    printf("PLACED BOX: PID:%d | WEIGHT:%d | TIME:%ld | LEFT WEIGHT ON ASSEMBLY LINE:%d | LEFT BOXES:%d\n",
           box.workerID, box.weight, box.loadTime.tv_usec, assemblyLine->maxWeight - assemblyLine->currentWeight,
           assemblyLine->maxBoxes - assemblyLine->currentBoxes);
    if (assemblyLine->truckEnded == 1 && assemblyLine->currentWeight == 0)
        exit(0);
}