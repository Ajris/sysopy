#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int signalsReceived = 0;
int numOfSignals;

void printError(char *message);

void blockSignals(char *mode);

void addHandlers(char *mode);

void handleEverything(int sig, siginfo_t *info, void *ucontext);

void sendSignals(char *mode, int numOfSignals, int catcherPID);

int main(int argc, char **argv) {
    if (argc != 4)
        printError("Wrong number of arguments");

    numOfSignals = atoi(argv[1]);
    int catcherPID = atoi(argv[2]);
    char *mode = argv[3];

    if (numOfSignals <= 0)
        printError("Wrong num of signals");
    if (catcherPID <= 0)
        printError("Wrong num of catcherpid");

    blockSignals(mode);
    sendSignals(mode, numOfSignals, catcherPID);
    addHandlers(mode);
    while (1);
}

void killAllWithProcess(int processNum, int signal, int endingSignal){
    for(int i = 0; i < numOfSignals; i++)
        kill(processNum, signal);
    kill(processNum, endingSignal);
}

void sendSignals(char *mode, int numOfSignals, int catcherPID) {
    if (strcmp(mode, "KILL") == 0) {
        killAllWithProcess(catcherPID, SIGUSR1, SIGUSR2);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        union sigval justToBeHere;
        for (int i = 0; i < numOfSignals; i++)
            sigqueue(catcherPID, SIGUSR1, justToBeHere);
        sigqueue(catcherPID, SIGUSR2, justToBeHere);
    } else if (strcmp(mode, "SIGRT") == 0) {
        killAllWithProcess(catcherPID, SIGRTMIN, SIGRTMAX);
    } else {
        printError("Wrong mode");
    }
}

void handle_KILL(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        signalsReceived++;
    } else {
        printf("Sender got: %d\n", signalsReceived);
        exit(1);
    }
}

void handle_SIGQUEUE(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        printf("This is: %d\n", info->si_value.sival_int);
        signalsReceived++;
    } else {
        printf("Sender got: %d\n", signalsReceived);
        exit(1);
    }
}

void handle_SIGRT(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGRTMIN) {
        signalsReceived++;
    } else {
        printf("Sender got: %d\n", signalsReceived);
        exit(1);
    }
}

void addHandlers(char* mode) {
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

void blockSignals(char *mode) {
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