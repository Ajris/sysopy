
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int value = 0;

void printError(char *message);
void ctrlZ_Handler(int sig, siginfo_t *siginfo, void *context);
void ctrlC_Handler(int signum);

int main(int argc, char **argv) {

    struct sigaction* act = malloc(sizeof(struct sigaction));
    act->sa_sigaction = &ctrlZ_Handler;
    act->sa_flags = SA_SIGINFO;
    sigaction(SIGTSTP, act, NULL);

    time_t t;
    signal(SIGINT, ctrlC_Handler);
    while(1){
        time(&t);
        printf("%s\n", ctime(&t));
        sleep(1);
    }
}
void ctrlZ_Handler(int sig, siginfo_t *siginfo, void *context){
    if(value == 0){
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu");
        value++;
    } else {
        value = 0;
    }
}

void ctrlC_Handler(int signum){
    printf("\nOdebrano sygnał SIGINT\n");
    exit(1);
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}