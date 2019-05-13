//
// Created by ajris on 12.05.19.
//
#include "both.h"

AssemblyLine* assemblyLine;

void printError(char *message);

int main(int argc, char **argv) {
    if(argc < 2)
        printError("Wrong num of arguments");

    int workers = atoi(argv[1]);

    int desc = shm_open("/sharedMemory", O_RDWR, 0);
    if(desc == -1)
        printError("ERROR WITH OPENING MEMORY");
    assemblyLine = mmap(NULL, sizeof(AssemblyLine), PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);

    if(assemblyLine == (void*)-1)
        printError("ERROR WITH GETTING ASSEMBLY LINE");

    for(int i = 0; i < workers; i++){
        if(fork() == 0){
            int m = atoi(argv[2 + i]);

            Box package;
            package.workerID = getpid();
            package.weight = m;

            if (argc > 2 + i + workers) {
                for (int j = 0; j < atoi(argv[2 + i + workers]); j++)
                    putBox(assemblyLine, package);
            } else {
                while (1) {
                    putBox(assemblyLine, package);
                }
            }
        }
    }

    for (int i = 0; i < workers; i++) {
        wait(NULL);
    }
}

void printError(char *message) {
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}