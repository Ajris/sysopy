#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/mman.h>

#define SHM_NAME "/kol_shm"
#define MAX_SIZE 1024

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }
    int desc = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
    ftruncate(desc, MAX_SIZE);
    int* t = mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);
    *t = atoi(argv[1]);
    /***************************************
    Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
    zapisz tam wartosc przekazana jako parametr wywolania programu
    posprzataj
    *****************************************/
    munmap(t, MAX_SIZE);
    return 0;
}
