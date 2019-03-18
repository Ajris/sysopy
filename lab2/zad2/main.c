#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ftw.h>

char *timeComparingType;
time_t inputTime;

void printError(char *message);

time_t parseTime(char *time);

int compareTime(time_t fileTime);

int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

void printFileInformation(const char *fpath, const struct stat *sb);

int main(int argc, char **argv) {
    if (argc != 5) {
        printError("Wrong number of arguments");
    }
    char *directory = realpath(argv[1], NULL);
    timeComparingType = argv[2];
    inputTime = parseTime(argv[3]);
    char *type = argv[4];

    if (strcmp(type, "nftw") == 0) {
        int res = nftw(directory, &fn, 100, FTW_PHYS);
        if (res == -1) {
            printError("Something wrong occured during nftw");
        }
    } else if (strcmp(type, "stat") == 0) {

    } else {
        printError("Wrong type specified");
    }
}

void printFileInformation(const char *fpath, const struct stat *sb) {
    printf("\nPATH: %s, SIZE: %ld\n", fpath, sb->st_size);
    printf("ACCESS TIME: %sMODIFICATION TIME: %s", ctime(&sb->st_atime), ctime(&sb->st_mtime));
    printf("TYPE: ");
    switch (sb->st_mode & S_IFMT) {
        case S_IFBLK:
            printf("urzadzenie blokowe");
            break;
        case S_IFCHR:
            printf("urzadzenie znakowe");
            break;
        case S_IFDIR:
            printf("katalog");
            break;
        case S_IFIFO:
            printf("potok nazwany");
            break;
        case S_IFLNK:
            printf("link symboliczny");
            break;
        case S_IFREG:
            printf("zwykly plik");
            break;
        case S_IFSOCK:
            printf("soket");
            break;
        default:
            printError("No idea what is this");
            break;
    }
    printf("\n");
}

int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (!compareTime(sb->st_mtime)) {
        printFileInformation(fpath, sb);
    }
    return 0;
}

int compareTime(time_t fileTime) {
    if (strcmp(timeComparingType, "<") == 0) {
        return fileTime < inputTime;
    } else if (strcmp(timeComparingType, ">") == 0) {
        return fileTime > inputTime;
    } else if (strcmp(timeComparingType, "=") == 0) {
        return fileTime == inputTime;
    } else {
        printError("Wrong type specified");
    }
    printError("Something went wrong");
    return 0;
}

time_t parseTime(char *time) {
    struct tm date;
    char *tmp = strptime(time, "%d-%m-%Y %H:%M:%S", &date);
    if (tmp == NULL) {
        printError("Error while parsing date");
    }
    time_t goodTime = mktime(&date);

    if (goodTime == -1) {
        printError("Error when mktime");
    }
    return goodTime;
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}