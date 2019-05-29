#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define KEY "./queuekey"

int main(int argc, char* argv[])
{

 if(argc !=2){
   printf("Not a suitable number of program parameters\n");
   return(1);
 }

 /******************************************************
 Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
 Wyslij do niej wartosc przekazana jako parametr wywolania programu
 Obowiazuja funkcje systemu V
 ******************************************************/
  key_t k = ftok(KEY, 1);
    int q = msgget(k, 0644);
    int val = atoi(argv[1]);

    struct msgbuf *buf = calloc(1, sizeof(long) + sizeof(val));

    buf->mtype = 1;

    memcpy(&buf->mtext, &val, sizeof(val));

    if(msgsnd(q, &buf, sizeof(val), 0) < 0){
        perror("msgsnd");
    }


  return 0;
}



