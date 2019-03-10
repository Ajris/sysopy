#ifndef ZAD1_LIBRARY_H
#define ZAD1_LIBRARY_H

#include <glob.h>

struct Result {
    char **blocks;
    size_t blockNum;
};

struct Result *createTable(size_t blockNum);

void freeTable(struct Result *table);

void searchFile(char *directory, char *fileToSearch, char *fileToSave);

void freeBlock(struct Result *table, int index);

int saveBlock(struct Result *table, char *file);

void printTable(struct Result *table);

#endif