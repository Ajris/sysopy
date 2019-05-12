//
// Created by ajris on 12.05.19.
//

#include "both.h"

int sharedMemoryID;
AssemblyLine *assemblyLine;
Truck truck;

void printError(char *message);

void handleCtrlC(int signum);

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

    if(assemblyLine == (void*)-1)
        printError("Error getting assemblyline");

    assemblyLine->maxWeight = maxAssemblyLineWeight;
    assemblyLine->maxBoxes = maxAssemblyLineBoxCount;
    assemblyLine->currentBoxes = 0;
    assemblyLine->currentWeight = 0;
    assemblyLine->currentBoxInLine = 0;
    truck.maxBoxCount = maxTruckBoxCount;


    if (shmdt(assemblyLine) == -1)
        printError("Error unlinking");
    if (shmctl(sharedMemoryID, IPC_RMID, NULL) == -1)
        printError("Error deleting");

    int semid = semget(getKey(), 2, 0666 | IPC_CREAT | IPC_EXCL);

    if (semid == -1) {
        printf("%d\n", errno);
        shmdt(assemblyLine);
        shmctl(semid, IPC_RMID, NULL);
        exit(-1);
    }
    printf("Empty truck arrived\n");

    assemblyLine->semaphoresID = semid;
    printf("Empty truck arrived\n");

    if (assemblyLine->semaphoresID == -1)
        printError("Error geting semaphoresID");

    semctl(assemblyLine->semaphoresID, LINE_SEMAPHORE, SETVAL, 0);
    semctl(assemblyLine->semaphoresID, TRUCK_SEMAPHORE, SETVAL, 0);

    signal(SIGINT, handleCtrlC);

    printf("Empty truck arrived\n");

    while (1) {
        getBox();
    }
}

void getBox() {
    int currentTaken = 0;
    while (1) {
        incSem(LINE_SEMAPHORE, assemblyLine);
        while (currentTaken != assemblyLine->currentBoxInLine) {
            Package newPackage = assemblyLine->line[currentTaken];
            time_t currentTime;
            time(&currentTime);
            truck.currentWeight += newPackage.weight;
            truck.currentBoxNumber++;
            printf("LOADED PACKAGE: PID:%d | WEIGHT:%d | TIMEDIFF:%ld | TRUCK WEIGHT:%d | LEFT BOXES:%d",
                   newPackage.workerID, newPackage.weight, newPackage.loadTime - currentTime, truck.currentWeight,
                   truck.maxBoxCount - truck.currentBoxNumber);
            currentTaken++;
        }
        decSem(LINE_SEMAPHORE, assemblyLine);
        tryToDecSem(TRUCK_SEMAPHORE, assemblyLine);
        printf("1\n");
    }
}

void handleCtrlC(int signum) {
    semctl(assemblyLine->semaphoresID, 0, IPC_RMID);
    shmctl(shmget(getKey(), 0, 0), IPC_RMID, NULL);
    exit(1);
}

void printError(char *message) {
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}