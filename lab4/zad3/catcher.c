#include <stdio.h>
#include <stdlib.h>

void printError(char *message);

int main(int argc, char **argv) {

    printf("1");
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}