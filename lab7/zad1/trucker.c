//
// Created by ajris on 12.05.19.
//

#include "both.h"

int sharedMemoryID;
AssemblyLine *assemblyLine;
Truck truck;

void printError(char *message);

void handleCtrlC(int signum);

void clearEverything();

void getBox();

int main(int argc, char **argv) {
    if (argc != 4)
        printError("Wrong num of arguments");

    int maxTruckBoxCount = atoi(argv[1]);
    int maxAssemblyLineBoxCount = atoi(argv[2]);
    int maxAssemblyLineWeight = atoi(argv[3]);

    if (maxTruckBoxCount <= 0 || maxAssemblyLineBoxCount <= 0 || maxAssemblyLineWeight <= 0)
        printError("Wrong argument");
    if (maxAssemblyLineBoxCount > MAX_BOXES_IN_ASSEMBLY_LINE)
        printError("Too many");

    if ((sharedMemoryID = shmget(getKey(), sizeof(AssemblyLine), IPC_CREAT | 0666 | IPC_EXCL)) < 0)
        printError("Error creating shared memory");

    assemblyLine = shmat(sharedMemoryID, NULL, 0);

    if (assemblyLine == (void *) -1)
        printError("Error getting assemblyline");

    assemblyLine->maxWeight = maxAssemblyLineWeight;
    assemblyLine->maxBoxes = maxAssemblyLineBoxCount;
    assemblyLine->currentBoxes = 0;
    assemblyLine->currentWeight = 0;
    assemblyLine->currentBoxInLine = 0;
    truck.maxBoxCount = maxTruckBoxCount;

    int semid = semget(getKey(), 3, 0666 | IPC_CREAT | IPC_EXCL);

    if (semid == -1) {
        printf("%d\n", errno);
        shmdt(assemblyLine);
        shmctl(semid, IPC_RMID, NULL);
        exit(-1);
    }

    assemblyLine->semaphoresID = semid;

    if (assemblyLine->semaphoresID == -1)
        printError("Error geting semaphoresID");

    semctl(assemblyLine->semaphoresID, END_LINE_SEMAPHORE, SETVAL, 0);
    semctl(assemblyLine->semaphoresID, START_LINE_SEMAPHORE, SETVAL, 1);
    semctl(assemblyLine->semaphoresID, TRUCK_SEMAPHORE, SETVAL, truck.maxBoxCount);

    signal(SIGINT, handleCtrlC);

    printf("Empty truck arrived\n");

    atexit(clearEverything);

    getBox();
}

void clearEverything(){
    semctl(assemblyLine->semaphoresID, 0, IPC_RMID);
    shmctl(shmget(getKey(), 0, 0), IPC_RMID, NULL);
}

void getBox() {
    int currentTaken = 0;
    int i = 0;
    while (1) {
        printf("Waiting for box\n");
        if (assemblyLine->truckEnded == 1){
            printf("Last Truck %d => Weight:%d | Boxes:%d\n", i, truck.currentWeight, truck.currentBoxNumber);
            exit(0);
        }
        takeSemaphore(END_LINE_SEMAPHORE, assemblyLine);
        if (assemblyLine->truckEnded == 1){
            printf("Last Truck %d => Weight:%d | Boxes:%d\n", i, truck.currentWeight, truck.currentBoxNumber);
            exit(0);
        }
//        if(tryToTakeSemaphore(END_LINE_SEMAPHORE, assemblyLine)){
        takeSemaphore(START_LINE_SEMAPHORE, assemblyLine);
        currentTaken %= MAX_BOXES_IN_ASSEMBLY_LINE;
        Box box = assemblyLine->line[currentTaken];
        truck.currentWeight += box.weight;
        truck.currentBoxNumber++;
        assemblyLine->currentBoxes--;
        assemblyLine->currentWeight -= box.weight;
        struct timeval end;
        gettimeofday(&end, NULL);
        printf("LOADED BOX: PID:%d | WEIGHT:%d | TIMEDIFF:%ld | TRUCK WEIGHT:%d | LEFT BOXES:%d\n",
               box.workerID, box.weight, end.tv_usec - box.loadTime.tv_usec, truck.currentWeight,
               truck.maxBoxCount - truck.currentBoxNumber);
        currentTaken++;

        if (truck.currentBoxNumber == truck.maxBoxCount) {
            printf("TRUCK IS FULL -> DEPARTURE STARTED\n");
            printf("Truck %d => Weight:%d | Boxes:%d\n", i, truck.currentWeight, truck.currentBoxNumber);
            truck.currentWeight = 0;
            truck.currentBoxNumber = 0;
            printf("Empty truck arrived\n");
            i++;
            struct sembuf s;
            s.sem_flg = 0;
            s.sem_num = TRUCK_SEMAPHORE;
            s.sem_op = truck.maxBoxCount;
            semop(assemblyLine->semaphoresID, &s, 1);
        }
        releaseSemaphore(START_LINE_SEMAPHORE, assemblyLine);
//        }
    }
}

void handleCtrlC(int signum) {
    assemblyLine->truckEnded = 1;
}

void printError(char *message) {
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}