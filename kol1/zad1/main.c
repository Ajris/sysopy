#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    FILE* file = fopen(argv[1], "r+");
//    fseek(file, 0, SEEK_END);
//    int size = ftell(file);
    fseek(file, -8, SEEK_END);
    char buf[8];
    if(fread(buf, 8, 1, file) == 8)
        printf("%s", buf);
    else
        printf("KURWA");
}
