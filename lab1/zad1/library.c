#include "library.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

struct Result *createTable(int blockNum) {
    if (blockNum <= 0) {
        return NULL;
    }
    struct Result *blocks = calloc(1, sizeof(struct Result));
    blocks->blockNum = blockNum;
    blocks->blocks = calloc(blockNum, sizeof(char *));
    for (int i = 0; i < blockNum; i++) {
        blocks->blocks[i] = NULL;
    }
    return blocks;
}

void freeTable(struct Result *table) {
    if (table != NULL) {
        if (table->blocks != NULL) {
            for(int i = 0; i < table->blockNum;i++){
                if(table->blocks[i] != NULL)
                    free(table->blocks[i]);
            }
            free(table->blocks);
        }
        free(table);
    }
}

void searchFile(char *directory, char *fileToSearch, char *fileToSave) {
    char *find = calloc(10000, sizeof(char));

    strcat(find, "find ");
    strcat(find, directory);
    strcat(find, " -name ");
    strcat(find, fileToSearch);
    strcat(find, " ");
    strcat(find, " > ");
    strcat(find, fileToSave);

    system(find);
}

void freeBlock(struct Result *table, int index) {
    if (table != NULL) {
        if (table->blockNum <= index) {
            printf("Index is not in table\n");
        } else {
            if (table->blocks[index] != NULL) {
                free(table->blocks[index]);
                table->blocks[index] = NULL;
            }
        }
    }
}

int saveBlock(struct Result *table, char *fileName) {
    if (table == NULL) {
        printf("Table wasnt initialized\n");
        return -2;
    }
    FILE *checkingSizeFile;
    size_t size = 0;
    checkingSizeFile = fopen(fileName, "r");

    if (checkingSizeFile == NULL) {
        printf("Couldnt open file\n");
        return -3;
    }

    fseek(checkingSizeFile, 0, SEEK_END);
    size = (size_t) ftell(checkingSizeFile);
    fclose(checkingSizeFile);

    FILE *readingFile = fopen(fileName, "r");
    if (readingFile == NULL) {
        printf("File not opened\n");
        return -2;
    } else if (size > 999999) {
        printf("File is too big\n");
        return -3;
    } else {
        char *content = calloc(size, sizeof(char));
        char *line = calloc(size, sizeof(char));
        while (fscanf(readingFile, "%s", line) != EOF) {
            strcat(content, line);
        }
        int savedIndex = -1;
        for (int j = 0; j < table->blockNum; j++) {
            if (table->blocks[j] == NULL) {
                savedIndex = j;
                table->blocks[j] = calloc(1, sizeof(char)*(size+1));
                table->blocks[j] = content;
                break;
            }
        }
        return savedIndex;
    }
    return -1;
}

void printTable(struct Result *table) {
    printf("TABLE: \n");
    for (int i = 0; i < table->blockNum; i++) {
        printf("\n%d:\n", i);
        if (table->blocks[i] != NULL) {
            printf("%s", table->blocks[i]);
        } else {
            printf("Nothing is here\n");
        }
    }
}