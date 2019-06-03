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

    sleep(1);
    int val = 0;
    int desc = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
    int* tmp = mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);
    val = *tmp;
    /*******************************************
    Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
    odczytaj zapisana tam wartosc i przypisz ja do zmiennej val
    posprzataj
    *********************************************/
    shm_unlink(SHM_NAME);
    printf("%d square is: %d \n", val, val * val);
    return 0;
}
