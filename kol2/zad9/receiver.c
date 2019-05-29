#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define KEY  "./queuekey"


int main() {
        sleep(1);
        int val = 0;


	/**********************************
	Otworz kolejke systemu V "reprezentowana" przez KEY
	**********************************/
    key_t k = ftok(KEY, 1);
    int q = msgget(k, IPC_CREAT | 0644);
    if(q < 0) {
    perror("msgget");
    exit(1);
    }


	while(1)
 	{
	    /**********************************
	    Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
	    obowiazuja funkcje systemu V
	    ************************************/

            if(msgrcv(q, &val, sizeof(val), 0, 0) < 0) {
                perror("Cannot receive message");
                exit(1);
            };
        	 printf("%d square is: %d\n", val, val*val);

	}

	/*******************************
	posprzataj
	********************************/
    msgctl(q, IPC_RMID, NULL);

     return 0;
   }
