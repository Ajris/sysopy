#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#include <dlfcn.h>

struct Clock {
    clock_t real;
    clock_t user;
    clock_t sys;
};

struct Result {
    char **blocks;
    size_t blockNum;
};

struct Result *(*createTable)(size_t blockNum);

void (*freeTable)(struct Result *table);

void (*searchFile)(char *directory, char *fileToSearch, char *fileToSave);

void (*freeBlock)(struct Result *table, int index);

int (*saveBlock)(struct Result *table, char *file);

void (*printTable)(struct Result *table);

/*
 RESULT* create_table INT numberOfBlocks
 VOID search_directory CHAR* directory CHAR* fileToSearch CHAR* fileToSave
 VOID remove_block INT indexOfBlock
 INT save_block CHAR* fileToSave
 VOID save_and_remove CHAR* fileToSave INT numOfOperations
 */

int main(int argc, char **argv) {
    void *library = dlopen("libzad3aDynamic.so", RTLD_LAZY);
    if (library == NULL) {
        printf("Error loading library");
        return 1;
    }
    *(void **) (&createTable) = dlsym(library, "createTable");
    *(void **) (&freeTable) = dlsym(library, "freeTable");
    *(void **) (&searchFile) = dlsym(library, "searchFile");
    *(void **) (&saveBlock) = dlsym(library, "saveBlock");
    *(void **) (&printTable) = dlsym(library, "printTable");
    *(void **) (&freeBlock) = dlsym(library, "freeBlock");

    if (dlerror() != NULL) {
        printf("Sth went wrong");
        return 1;
    }

    struct tms *begin = malloc(sizeof(struct tms));
    struct tms *end = malloc(sizeof(struct tms));
    struct Clock savedBegin;
    struct Clock savedEnd;

    struct Result *result = NULL;
    printf("REAL - SYS - USER\n");
    for (int i = 1; i < argc; i++) {
        savedBegin.real = times(begin);
        savedBegin.sys = begin->tms_stime;
        savedBegin.user = begin->tms_utime;
        if (strcmp(argv[i], "create_table") == 0) {
            if (i + 1 >= argc) {
                printf("Error in input, not enough parameters for creating table");
                return 1;
            }
            char *tmp;
            result = createTable((size_t) strtol(argv[i + 1], &tmp, 0));
            i = i + 1;
        } else if (strcmp(argv[i], "search_directory") == 0) {
            if (i + 3 >= argc) {
                printf("Error in input, not enough parameters for search");
                return 1;
            }
            char *directory = argv[i + 1];
            char *fileToSearch = argv[i + 2];
            char *fileToSave = argv[i + 3];
            searchFile(directory, fileToSearch, fileToSave);
            i = i + 3;
        } else if (strcmp(argv[i], "remove_block") == 0) {
            if (i + 1 >= argc) {
                printf("Error in input, not enough parameters for creating table");
                return 1;
            }
            char *tmp;
            int num = (int) strtol(argv[i + 1], &tmp, 0);
            freeBlock(result, num);
            i = i+1;
        } else if (strcmp(argv[i], "save_block") == 0) {
            if (i + 1 >= argc) {
                printf("Error in input, not enough parameters for creating table");
                return 1;
            }
            int index = saveBlock(result, argv[i + 1]);
            if (index >= 0) {
//                printf("Saved on Index: %d\n", index);
            } else if (index == -1) {
                printf("Sorry there were no place for another block\n");
            }
            i = i + 1;
        } else if (strcmp(argv[i], "save_and_remove") == 0) {
            char *tmp;
            if (i + 2 >= argc) {
                return 1;
            }
            int num = (int) strtol(argv[i + 2], &tmp, 0);

            for (int j = 0; j < num; j++) {
                int index = saveBlock(result, argv[i + 1]);
                if (result->blocks[index] != NULL) {
                    freeBlock(result, index);
                }
            }
            i = i + 2;
        }

        savedEnd.real = times(end);
        savedEnd.sys = end->tms_stime;
        savedEnd.user = end->tms_utime;
        printf("%lf - %lf - %lf\n",
               (double) (savedEnd.real - savedBegin.real) / sysconf(_SC_CLK_TCK),
               (double) (savedEnd.sys - savedBegin.sys) / sysconf(_SC_CLK_TCK),
               (double) (savedEnd.user - savedBegin.user) / sysconf(_SC_CLK_TCK));
    }
    if (result != NULL) {
//        printTable(result);
        freeTable(result);
    }
    dlclose(library);
}
