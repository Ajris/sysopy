//
// Created by ajris on 12.05.19.
//


#include "both.h"

key_t getKey(){
    return ftok(getenv("HOME"), PROJECT_ID);
}

void incSem(int semaphore, AssemblyLine* assemblyLine){
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = 1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

void decSem(int semaphore, AssemblyLine* assemblyLine){
    struct sembuf s;
    s.sem_flg = 0;
    s.sem_num = semaphore;
    s.sem_op = -1;
    semop(assemblyLine->semaphoresID, &s, 1);
}

void tryToDecSem(int semaphore, AssemblyLine* assemblyLine){
    struct sembuf s;
    s.sem_flg = IPC_NOWAIT;
    s.sem_num = semaphore;
    s.sem_op = 0;
    semop(assemblyLine->semaphoresID, &s, 1);
}