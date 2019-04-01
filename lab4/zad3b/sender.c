#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int signalsReceived = 0;
int numOfSignals;
char* mode;
int catcherPID;

void printError(char *message);

void blockSignals();

void addHandlers();

void handle_KILL(int sig, siginfo_t *info, void *ucontext);

void handle_SIGQUEUE(int sig, siginfo_t *info, void *ucontext);

void handle_SIGRT(int sig, siginfo_t *info, void *ucontext);

void killAllWithProcess(int processNum, int signal, int endingSignal);

void sendSignals();

int main(int argc, char **argv) {
    if (argc != 4)
        printError("Wrong number of arguments");

    numOfSignals = atoi(argv[1]);
    catcherPID = atoi(argv[2]);
    mode = argv[3];

    if (numOfSignals <= 0)
        printError("Wrong num of signals");
    if (catcherPID <= 0)
        printError("Wrong num of catcherpid");

    blockSignals();
    sendSignals();
    addHandlers();
    while (1);
}

void sendSignals() {
    if (strcmp(mode, "KILL") == 0) {
        kill(catcherPID, SIGUSR1);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval justToBeHere;
        sigqueue(catcherPID, SIGUSR1, justToBeHere);
    } else if (strcmp(mode, "SIGRT") == 0) {
        kill(catcherPID, SIGRTMIN);
    } else {
        printError("Wrong mode");
    }
}

void handle_KILL(int sig, siginfo_t *info, void *ucontext){
    if(sig == SIGUSR1){
        signalsReceived++;
        if(signalsReceived < numOfSignals){
            kill(catcherPID, SIGUSR1);
        } else {
            kill(catcherPID, SIGUSR2);
        }
    } else {
        printf("Sent: %d Got: %d\n", numOfSignals, signalsReceived);
        exit(1);
    }
}

void handle_SIGQUEUE(int sig, siginfo_t *info, void *ucontext){
    if(sig == SIGUSR1){
        signalsReceived++;
        if(signalsReceived < numOfSignals){
            union sigval justToBeHere;
            sigqueue(catcherPID, SIGUSR1, justToBeHere);
        } else {
            union sigval justToBeHere;
            sigqueue(catcherPID, SIGUSR2, justToBeHere);
        }
        union sigval justToBeHere;
        sigqueue(info->si_pid, SIGUSR1, justToBeHere);
    } else {
        printf("Sent: %d Got: %d\n", numOfSignals, signalsReceived);
        exit(1);
    }
}

void handle_SIGRT(int sig, siginfo_t *info, void *ucontext){
    if(sig == SIGRTMIN){
        signalsReceived++;
        if(signalsReceived < numOfSignals){
            kill(catcherPID, SIGRTMIN);
        } else {
            kill(catcherPID, SIGRTMAX);
        }
    } else {
        printf("Sent: %d Got: %d\n", numOfSignals, signalsReceived);
        exit(1);
    }
}

void addHandlers() {
    struct sigaction *handlerInfos = malloc(sizeof(struct sigaction));
    handlerInfos->sa_flags = SA_SIGINFO;
    sigemptyset(&handlerInfos->sa_mask);
    if (strcmp(mode, "KILL") == 0) {
        handlerInfos->sa_sigaction = handle_KILL;
        sigaction(SIGUSR1, handlerInfos, NULL);
        sigaction(SIGUSR2, handlerInfos, NULL);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        handlerInfos->sa_sigaction = handle_SIGQUEUE;
        sigaction(SIGUSR1, handlerInfos, NULL);
        sigaction(SIGUSR2, handlerInfos, NULL);
    } else if (strcmp(mode, "SIGRT") == 0) {
        handlerInfos->sa_sigaction = handle_SIGRT;
        sigaction(SIGRTMIN, handlerInfos, NULL);
        sigaction(SIGRTMAX, handlerInfos, NULL);
    } else {
        printError("wrong mode");
    }
}

void blockSignals() {
    sigset_t *allSignalsToBlock = malloc(sizeof(sigset_t));
    sigfillset(allSignalsToBlock);
    if (strcmp(mode, "KILL") == 0) {
        sigdelset(allSignalsToBlock, SIGUSR1);
        sigdelset(allSignalsToBlock, SIGUSR2);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigdelset(allSignalsToBlock, SIGUSR1);
        sigdelset(allSignalsToBlock, SIGUSR2);
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigdelset(allSignalsToBlock, SIGRTMIN);
        sigdelset(allSignalsToBlock, SIGRTMAX);
    } else {
        printError("wrong mode");
    }
    sigprocmask(SIG_BLOCK, allSignalsToBlock, NULL);
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}