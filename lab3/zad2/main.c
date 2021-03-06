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
    int monitoringTime;
    int type;
};

struct fileData {
    char *path;
    int repeatTime;
};


void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readLinesFromFile(char *filename);

void createProcesses(struct fileData **fileData, struct input *input);

int main(int argc, char **argv) {
    if (argc != 4) {
        printError("Wrong number of arguments");
    }
    struct input *input = parseArguments(argv);
    struct fileData **fileData = readLinesFromFile(input->filename);
    createProcesses(fileData, input);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

void watchCopyNewFile(struct fileData *fileData, struct input *input) {
    time_t startTime = time(NULL);
    time_t endTime = startTime + input->monitoringTime;
    time_t currentTime = startTime;
    int numOfCopies = 0;

    struct stat fileStats;
    if (lstat(fileData->path, &fileStats) == -1) {
        printf("File: %s", fileData->path);
        printError("Coudlnt read file");
    }

    time_t lastModification = fileStats.st_mtime;
    while (currentTime <= endTime) {
        if (lstat(fileData->path, &fileStats) == -1) {
            printf("File: %s", fileData->path);
            printError("Coudlnt read file");
        }
        char *modificationTime = malloc(sizeof(char) * 1000);
        strftime(modificationTime, 1000, "_%Y-%m-%d_%H-%M-%S", localtime(&fileStats.st_mtime));
        char *newFileName = malloc(1000 * sizeof(char));
        sprintf(newFileName, "%s%s", fileData->path, modificationTime);

        if (lastModification < fileStats.st_mtime || numOfCopies == 0) {
            lastModification = fileStats.st_mtime;
            pid_t newProc = vfork();
            if (newProc == 0) {
                execlp("cp", "cp", fileData->path, newFileName, NULL);
            } else if (newProc == -1) {
                printError("Something went wrong");
            } else {
                int status;
                wait(&status);
                if (status != 0) {
                    printError("Something went wrong");
                } else {
                    numOfCopies++;
                }
            }
        }
        sleep((unsigned int) fileData->repeatTime);
        currentTime = time(NULL);
        free(modificationTime);
        free(newFileName);
    }
    exit(numOfCopies);
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

char* getOnlyFileName(char* filePath){
    char* fileName = strchr(filePath, '/');
    if(fileName == NULL){
        printError("Something went wrong with parsing name of file");
    }
    return fileName + 1;
}

void watchFileToMemory(struct fileData *fileData, struct input *input) {
    time_t startTime = time(NULL);
    time_t endTime = startTime + input->monitoringTime;
    time_t currentTime = startTime;
    int numOfCopies = 0;

    struct stat fileStats;
    if (lstat(fileData->path, &fileStats) == -1) {
        printf("File: %s", fileData->path);
        printError("Coudlnt read file");
    }

    time_t lastModification = fileStats.st_mtime;

    char *content = getContent(fileData->path);

    while (currentTime <= endTime) {
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
            if(!dir){
                mkdir("archiwum", 0777);
            }
            FILE *file = fopen(newFileName, "w");
            if(!file){
                printError("Coudlnt created file");
            }
            fwrite(content, 1, strlen(content), file);
            fclose(file);
            lastModification = fileStats.st_mtime;
            content = getContent(fileData->path);
            numOfCopies++;
            closedir(dir);
        }
        sleep((unsigned int) fileData->repeatTime);
        currentTime = time(NULL);
        free(modificationTime);
        free(newFileName);
    }
    free(content);
    exit(numOfCopies);
}

void createProcesses(struct fileData **fileData, struct input *input) {
    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = fork();
        if (curr == 0)
            if (input->type == 0) {
                watchCopyNewFile(fileData[i], input);
            } else if (input->type == 1) {
                watchFileToMemory(fileData[i], input);
            } else {
                printError("Unknown type");
            }
        else {
        }
    }
    int *tmp = malloc(sizeof(int));

    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = wait(tmp);
        printf("Process %d created %d file copies %s.\n", curr, WEXITSTATUS(tmp[0]), fileData[i]->path);
    }
    free(tmp);
}

struct fileData **readLinesFromFile(char *filename) {
    FILE *file = fopen(fileName, "r");
    struct fileData **fileData = malloc(sizeof(struct fileData *) * MAX_FILE_NUM);
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        fileData[i] = malloc(sizeof(struct fileData) + sizeof(char) * MAX_FILELINE);
    }

    if (!file)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_FILELINE);

    while (fgets(currentLine, MAX_FILELINE, file) != NULL) {
        char *token = strtok(currentLine, " ");
        fileData[numOfFiles]->path = strdup(token);
        char *ptr = strtok(NULL, " ");
        fileData[numOfFiles]->repeatTime = atoi(ptr);
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
    input->monitoringTime = atoi(argv[2]);
    input->type = atoi(argv[3]);
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(0);
}