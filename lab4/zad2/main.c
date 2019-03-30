#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_FILE_NUM 100
#define MAX_FILELINE 100

int numOfFiles = 0;

struct input {
    char *filename;
};

struct fileData {
    char *path;
    int pid;
    int stopped;
    int repeatingTime;
};

void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readFromFile(char *filename);

void createProcesses(struct fileData **fileData, struct input *input);

int main(int argc, char **argv) {
    if (argc != 2) {
        printError("Wrong number of arguments");
    }
    struct input *input = parseArguments(argv);
    struct fileData **fileData = readFromFile(input->filename);
    createProcesses(fileData, input);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}


char *getContent(char *filename) {
    struct stat fileinfo;
    if (lstat(filename, &fileinfo) != 0) {
        printError("Couldnt read file");
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printError("Couldnt read file");
    }

    char *content = malloc(fileinfo.st_size + 1);
    if (fread(content, 1, fileinfo.st_size, file) != fileinfo.st_size) {
        printError("Could not read file");
    }

    content[fileinfo.st_size] = '\0';
    fclose(file);
    return content;
}

char *getOnlyFileName(char *filePath) {
    char *fileName = strchr(filePath, '/');
    if (fileName == NULL) {
        printError("Something went wrong with parsing name of file");
    }
    return fileName + 1;
}

void watchFileToMemory(struct fileData *fileData, struct input *input) {
    int numOfCopies = 0;

    struct stat fileStats;
    if (lstat(fileData->path, &fileStats) == -1) {
        printf("File: %s", fileData->path);
        printError("Coudlnt read file");
    }

    time_t lastModification = fileStats.st_mtime;

    char *content = getContent(fileData->path);
    while (1) {
        if (lstat(fileData->path, &fileStats) == -1) {
            printf("File: %s", fileData->path);
            printError("Coudlnt read file");
        }
        char *modificationTime = malloc(sizeof(char) * 1000);
        strftime(modificationTime, 1000, "_%Y-%m-%d_%H-%M-%S", localtime(&fileStats.st_mtime));
        char *newFileName = malloc(1000 * sizeof(char));
        sprintf(newFileName, "archiwum/%s%s", getOnlyFileName(fileData->path), modificationTime);
        if (lastModification < fileStats.st_mtime) {
            DIR *dir = opendir("archiwum");
            if (!dir) {
                mkdir("archiwum", 0777);
            }
            FILE *file = fopen(newFileName, "w");
            if (!file) {
                printError("Coudlnt created file");
            }
            fwrite(content, 1, strlen(content), file);
            fclose(file);
            lastModification = fileStats.st_mtime;
            content = getContent(fileData->path);
            numOfCopies++;
            closedir(dir);
        }
        free(modificationTime);
        free(newFileName);
        sleep(fileData->repeatingTime);
    }
    free(content);
}

void createProcesses(struct fileData **fileData, struct input *input) {
    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = fork();
        if (curr == 0) {
            fileData[i]->pid = getpid();
            watchFileToMemory(fileData[i], input);
        } else {
            fileData[i]->pid = curr;
        }
    }
    int *tmp = malloc(sizeof(int));

    while (1) {
        char *value = malloc(sizeof(char) * 64);
        fgets(value, 10, stdin);
        if (strcmp(value, "END\n") == 0) {
            for (int i = 0; i < numOfFiles; i++) {
                int num = kill(fileData[i]->pid, SIGKILL);
                printf("Process %d created %d file copies of %s.\n", fileData[i]->pid, WEXITSTATUS(tmp[0]),
                       fileData[i]->path);
                printf("Num %d\n", num);
            }
            free(value);
            exit(1);
        }
    }
}

struct fileData **readFromFile(char *filename) {
    FILE *file = fopen(filename, "r");
    struct fileData **fileData = malloc(sizeof(struct fileData *) * MAX_FILE_NUM);
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        fileData[i] = malloc(sizeof(struct fileData) + 2 * sizeof(int) + sizeof(char) * MAX_FILELINE);
    }

    if (!file)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_FILELINE);

    while (fgets(currentLine, MAX_FILELINE, file) != NULL) {
        char *token = strtok(currentLine, " ");
        fileData[numOfFiles]->path = strdup(token);
        char *ptr = strtok(NULL, " ");
        fileData[numOfFiles]->repeatingTime = atoi(ptr);
        ptr = strtok(NULL, " ");
        if (ptr != NULL)
            printError("Wrong number of arguments near file");
        numOfFiles++;

        if (numOfFiles > MAX_FILE_NUM)
            printError("Too many files added in lista");
    }

    fclose(file);
    return fileData;
}

struct input *parseArguments(char **argv) {
    struct input *input = malloc(sizeof(struct input));
    input->filename = argv[1];
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(0);
}