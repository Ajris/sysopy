//
// Created by ajris on 12.05.19.
//

#include "both.h"

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

    int desc = shm_open("/sharedMemory", O_CREAT | O_RDWR , 0666);
    if (desc < 0)
        printError("ERROR");
    if (ftruncate(desc, sizeof(AssemblyLine)) == -1)
        printError("ERROR");

    assemblyLine = mmap(NULL, sizeof(AssemblyLine), PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);

    if (assemblyLine == (void *) -1)
        printError("Error getting assemblyline");

    assemblyLine->maxWeight = maxAssemblyLineWeight;
    assemblyLine->maxBoxes = maxAssemblyLineBoxCount;
    assemblyLine->currentBoxes = 0;
    assemblyLine->currentWeight = 0;
    assemblyLine->currentBoxInLine = 0;
    truck.maxBoxCount = maxTruckBoxCount;

    semaphores[END_LINE_SEMAPHORE] = sem_open("/END_LINE_SEMAPHORE", O_RDWR | O_CREAT, 0666, 0);
    semaphores[TRUCK_SEMAPHORE] = sem_open("/TRUCK_SEMAPHORE", O_RDWR | O_CREAT, 0666, truck.maxBoxCount);
    semaphores[START_LINE_SEMAPHORE] = sem_open("/START_LINE_SEMAPHORE", O_RDWR | O_CREAT, 0666, 1);

    signal(SIGINT, handleCtrlC);

    printf("Empty truck arrived\n");

    atexit(clearEverything);

    getBox();
}

void clearEverything() {
    sem_unlink("/END_LINE_SEMAPHORE");
    sem_unlink("/TRUCK_SEMAPHORE");
    sem_unlink("/START_LINE_SEMAPHORE");
    shm_unlink("/sharedMemory");
}

void getBox() {
    int currentTaken = 0;
    int i = 0;
    while (1) {
        printf("Waiting for box\n");
        if (assemblyLine->truckEnded == 1) {
            printf("Last Truck %d => Weight:%d | Boxes:%d\n", i, truck.currentWeight, truck.currentBoxNumber);
            exit(0);
        }
        takeSemaphore(END_LINE_SEMAPHORE);
        if (assemblyLine->truckEnded == 1) {
            printf("Last Truck %d => Weight:%d | Boxes:%d\n", i, truck.currentWeight, truck.currentBoxNumber);
            exit(0);
        }
        takeSemaphore(START_LINE_SEMAPHORE);
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
            for(int j = 0; j < truck.maxBoxCount; j++)
                releaseSemaphore(TRUCK_SEMAPHORE);
        }
        releaseSemaphore(START_LINE_SEMAPHORE);
    }
}

void handleCtrlC(int signum) {
    assemblyLine->truckEnded = 1;
    printf("Last => Weight:%d | Boxes:%d\n", truck.currentWeight, truck.currentBoxNumber);
    exit(0);
}

void printError(char *message) {
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}