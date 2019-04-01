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

void handle_KILL(int sig, siginfo_t *info, void *ucontext);

void handle_SIGQUEUE(int sig, siginfo_t *info, void *ucontext);

void handle_SIGRT(int sig, siginfo_t *info, void *ucontext);

void killAllWithProcess(int processNum, int signal, int endingSignal);

int main(int argc, char **argv) {
    printf("Catcher PID: %d\n", getpid());
    if (argc != 2) {
        printError("Wrong num of arguments");
    }
    mode = argv[1];
    addHandlers();
    blockSignals();
    while (1);
}

void killAllWithProcess(int processNum, int signal, int endingSignal) {
    for (int i = 0; i < signalsReceived; i++)
        kill(processNum, signal);
    kill(processNum, endingSignal);
}

void handle_KILL(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        signalsReceived++;
    } else {
        killAllWithProcess(info->si_pid, SIGUSR1, SIGUSR2);
        printf("Catcher got: %d\n", signalsReceived);
        exit(1);
    }
}

void handle_SIGQUEUE(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        signalsReceived++;
    } else {
        union sigval justToBeHere;
        for (int i = 0; i < signalsReceived; i++) {
            justToBeHere.sival_int = i;
            sigqueue(info->si_pid, SIGUSR1, justToBeHere);
        }
        justToBeHere.sival_int = signalsReceived - 1;
        sigqueue(info->si_pid, SIGUSR2, justToBeHere);
        printf("Catcher got: %d\n", signalsReceived);
        exit(1);
    }
}

void handle_SIGRT(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGRTMIN) {
        signalsReceived++;
    } else {
        killAllWithProcess(info->si_pid, SIGRTMIN, SIGRTMAX);
        printf("Catcher got: %d\n", signalsReceived);
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