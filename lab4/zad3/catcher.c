#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int signalsReceived = 0;
char *mode;

void printError(char *message);

void blockSignals();

void addHandlers();

void handleEverything(int sig, siginfo_t *info, void *ucontext);

int main(int argc, char **argv) {
    if (argc != 2)
        printError("Wrong number of arguments");
    printf("Catcher PID: %d", getpid());
    mode = argv[1];
    blockSignals();
    addHandlers();
    while (1);
}

void handleEverything(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1 || sig == SIGRTMIN) {
        signalsReceived++;
    } else if (sig == SIGUSR2 || sig == SIGRTMAX) {
        int processNum = info->si_pid;
        if (strcmp(mode, "KILL") == 0) {
            for (int i = 0; i < signalsReceived; i++)
                kill(processNum, SIGUSR1);
            kill(processNum, SIGUSR2);
        } else if (strcmp(mode, "SIGQUEUE") == 0) {
            union sigval justToBeHere;
            for (int i = 0; i < signalsReceived; i++)
                sigqueue(processNum, SIGUSR1, justToBeHere);
            sigqueue(processNum, SIGUSR2, justToBeHere);
        } else if (strcmp(mode, "SIGRT") == 0) {
            for (int i = 0; i < signalsReceived; i++)
                kill(processNum, SIGRTMIN);
            kill(processNum, SIGRTMAX);
        } else {
            printError("Sth went wrong");
        }
        printf("Catcher got: %d\n", signalsReceived);
        exit(1);
    }
}

void addHandlers() {
    struct sigaction *handlerInfos = malloc(sizeof(struct sigaction));
    handlerInfos->sa_flags = SA_SIGINFO;
    handlerInfos->sa_sigaction = handleEverything;
    sigemptyset(&handlerInfos->sa_mask);
    if (strcmp(mode, "KILL") == 0) {
        sigaddset(&handlerInfos->sa_mask, SIGUSR1);
        sigaddset(&handlerInfos->sa_mask, SIGUSR2);
        sigaction(SIGUSR1, handlerInfos, NULL);
        sigaction(SIGUSR2, handlerInfos, NULL);
    } else if (strcmp(mode, "SIGQUEUE") == 0) {
        sigaddset(&handlerInfos->sa_mask, SIGUSR1);
        sigaddset(&handlerInfos->sa_mask, SIGUSR2);
        sigaction(SIGUSR1, handlerInfos, NULL);
        sigaction(SIGUSR2, handlerInfos, NULL);
    } else if (strcmp(mode, "SIGRT") == 0) {
        sigaddset(&handlerInfos->sa_mask, SIGRTMIN);
        sigaddset(&handlerInfos->sa_mask, SIGRTMAX);
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