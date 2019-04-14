#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	int pdesk[2];
	if (pipe(pdesk) == -1){
		perror("PIPE");
		return 1;
	}

	switch(fork()){
	  case -1:
		perror("FORK");
		return 2;
	  case 0:
		dup2(pdesk[1], STDOUT_FILENO);
		execvp("ls", argv);
		perror("EXECVP ls");
		exit(1);
	  default: {
   /* Zamknij deskryptor do zapisu,
    * skopiuj deskyptor końca do odczytu tego potoku na deskryptor wejścia standardowego,
    * wykonaj  tr "a-z" "A-Z", w przypadku błędu  obsłuż go i wyjdź, zwracając 3.
   */







   /* koniec */
       	 }
	}
   return 0;
   }
