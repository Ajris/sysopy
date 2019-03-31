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

void handleEverything(int sig, siginfo_t *info, void *ucontext);

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

void handleEverything(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1 || sig == SIGRTMIN) {
        signalsReceived++;
        if(signalsReceived < numOfSignals){
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
        } else {
            if (strcmp(mode, "KILL") == 0) {
                kill(catcherPID, SIGUSR2);
            } else if (strcmp(mode, "SIGQUEUE") == 0) {
                union sigval justToBeHere;
                sigqueue(catcherPID, SIGUSR2, justToBeHere);
            } else if (strcmp(mode, "SIGRT") == 0) {
                kill(catcherPID, SIGRTMAX);
            } else {
                printError("Wrong mode");
            }
        }
    } else if (sig == SIGUSR2 || sig == SIGRTMAX) {
        printf("Sent: %d Got: %d\n", numOfSignals, signalsReceived);
        exit(1);
    }
}

void addHandlers() {
    struct sigaction *act = malloc(sizeof(struct sigaction));
    act->sa_flags = SA_SIGINFO;
    act->sa_sigaction = handleEverything;
    sigemptyset(&act->sa_mask);
    if (strcmp(mode, "KILL") == 0) {
        sigaddset(&act->sa_mask, SIGUSR1);
        sigaddset(&act->sa_mask, SIGUSR2);
        sigaction(SIGUSR1, act, NULL);
        sigaction(SIGUSR2, act, NULL);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigaddset(&act->sa_mask, SIGUSR1);
        sigaddset(&act->sa_mask, SIGUSR2);
        sigaction(SIGUSR1, act, NULL);
        sigaction(SIGUSR2, act, NULL);
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigaddset(&act->sa_mask, SIGRTMIN);
        sigaddset(&act->sa_mask, SIGRTMAX);
        sigaction(SIGRTMIN, act, NULL);
        sigaction(SIGRTMAX, act, NULL);
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