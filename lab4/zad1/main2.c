
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

int value = 0;

void printError(char *message);
void ctrlZ_Handler(int sig, siginfo_t *siginfo, void *context);
void ctrlC_Handler(int signum);

int processPID;

int main(int argc, char **argv) {
    struct sigaction* act = malloc(sizeof(struct sigaction));
    act->sa_sigaction = &ctrlZ_Handler;
    act->sa_flags = SA_SIGINFO;
    sigaction(SIGTSTP, act, NULL);
    signal(SIGINT, ctrlC_Handler);

    if (!(processPID = fork())) {
        execl("script.sh", "script.sh", NULL);
        exit(0);
    }
    while(1) {}
}
void ctrlZ_Handler(int sig, siginfo_t *siginfo, void *context){
    if(!waitpid(processPID, NULL, WNOHANG)) {
        kill(processPID, SIGKILL);
    } else {
        if (!(processPID = fork())) {
            execl("script.sh", "script.sh", NULL);
        }
    }
}

void ctrlC_Handler(int signum){
    if(!waitpid(processPID, NULL, WNOHANG))
        kill(processPID, SIGKILL);
    exit(1);
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}