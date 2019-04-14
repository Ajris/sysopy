#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int sig, siginfo_t *info, void *ucontext){
    printf("ARGUMENT: %d", info->si_value.sival_int);
    exit(0);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &sighandler;

    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    sigaction(SIGUSR1, &action, NULL);

    int child = fork();
    if (child == 0) {
//        action.sa_mask = sigset;
        // => zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
    } else {
        union sigval sigval;
        sigval.sival_int = atoi(argv[1]);
        sigqueue(child, atoi(argv[2]), sigval);
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
    }

    return 0;
}
