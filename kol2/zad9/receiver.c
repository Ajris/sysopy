#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define KEY  "/home/ajris"

struct mess {
    long mtype;
    long value;
};

int main() {
    sleep(1);
    /**********************************
    Otworz kolejke systemu V "reprezentowana" przez KEY
    **********************************/
    key_t key = ftok(KEY, 0);
    int queueID = msgget(key, 0);
    if (queueID < 0) {
        perror("msgget");
        exit(1);
    }
    /**********************************
    Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
    obowiazuja funkcje systemu V
    ************************************/
    struct mess m;
    int val;
    if (msgrcv(queueID, &m, sizeof(long), -100, 0) < 0) {
        perror("Cannot receive message");
        exit(1);
    }
    val = m.value;
    printf("%d square is: %d\n", val, val * val);
    /*******************************
    posprzataj
    ********************************/
    msgctl(queueID, IPC_RMID, NULL);

    return 0;
}
