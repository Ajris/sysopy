//
// Created by ajris on 19.05.19.
//

#include "info.h"

Filter *filterMatrix;
Image *imageMatrix;
Image *outputMatrix;
int threadNum;

void printError(char *message);

void createOutImage(FILE *outFile);

long getCurrentTime();

void *blockMode(void *id);

void *interleavedMode(void *id);

void getOutMatrix();

void getImageMatrix(FILE *imageFile);

void getFilterMatrix(FILE *filterFile);

int main(int argc, char **argv) {
    long start = getCurrentTime();

    if (argc != 6)
        printError("Wrong num of arguments");

    threadNum = atoi(argv[1]);
    char *type = argv[2];
    char *imageFilename = argv[3];
    char *filterFilename = argv[4];
    char *outputFilename = argv[5];

    printf("THREAD NUMBER: %d ", threadNum);
    printf("MODE: %s ", type);
    printf("FILTER: %s ", filterFilename);
    printf("IMAGEFILE: %s ", imageFilename);
    printf("OUTPUTFILE: %s\n", outputFilename);

    FILE *imageFile = fopen(imageFilename, "r");
    getImageMatrix(imageFile);

    FILE *filterFile = fopen(filterFilename, "r");
    getFilterMatrix(filterFile);

    getOutMatrix();

    pthread_t threads[threadNum];

    for (int i = 0; i < threadNum; i++) {
        int *val = malloc(sizeof(int));
        *val = i;
        pthread_t thread;
        if (!strcmp(type, "block")) {
            pthread_create(&thread, NULL, &blockMode, val);
        } else if (!strcmp(type, "interleaved")) {
            pthread_create(&thread, NULL, &interleavedMode, val);
        } else {
            printError("Wrong type");
        }
        threads[i] = thread;
    }

    for (int i = 0; i < threadNum; i++) {
        long *time;
        pthread_join(threads[i], (void *) &time);
        printf(" %i: %ld |", i, *time);
    }

    FILE *outputFile = fopen(outputFilename, "w");
    createOutImage(outputFile);

    printf("\nFull time: %ld\n", getCurrentTime() - start);

    return 0;
}

void getFilterMatrix(FILE *filterFile) {
    if (filterFile == NULL)
        printError("ERROR");
    int size;
    filterMatrix = malloc(sizeof(Filter));
    fscanf(filterFile, "%i", &size);
    filterMatrix->size = size;
    filterMatrix->matrix = malloc(filterMatrix->size * sizeof(double *));
    for (int i = 0; i < filterMatrix->size; i++) {
        filterMatrix->matrix[i] = malloc(filterMatrix->size * sizeof(double));
    }

    for (int x = 0; x < filterMatrix->size; x++) {
        for (int y = 0; y < filterMatrix->size; y++) {
            fscanf(filterFile, "%lf", &filterMatrix->matrix[x][y]);
        }
    }

    fclose(filterFile);
}

void getImageMatrix(FILE *imageFile) {
    int width;
    int height;
    if (imageFile == NULL)
        printError("ERROR");
    fscanf(imageFile, "P2 %i %i 255", &width, &height);
    imageMatrix = malloc(sizeof(imageMatrix));
    imageMatrix->width = width;
    imageMatrix->height = height;

    imageMatrix->matrix = malloc(imageMatrix->height * sizeof(int *));
    for (int i = 0; i < imageMatrix->height; i++) {
        imageMatrix->matrix[i] = malloc(imageMatrix->width * sizeof(int));
    }

    for (int y = 0; y < imageMatrix->height; y++) {
        for (int x = 0; x < imageMatrix->width; x++) {
            fscanf(imageFile, "%i", &imageMatrix->matrix[y][x]);
        }
    }

    fclose(imageFile);
}

void getOutMatrix() {
    outputMatrix = malloc(sizeof(outputMatrix));
    outputMatrix->height = imageMatrix->height;
    outputMatrix->width = imageMatrix->width;
    outputMatrix->matrix = malloc(outputMatrix->height * sizeof(int *));
    for (int i = 0; i < outputMatrix->height; i++) {
        outputMatrix->matrix[i] = malloc(outputMatrix->width * sizeof(int));
        for (int j = 0; j < outputMatrix->width; j++) {
            outputMatrix->matrix[i][j] = 0;
        }
    }
}

int max(int first, int second) {
    return first > second ? first : second;
}

int min(int first, int second){
    return first > second ? second : first;
}

void filter(int wid, int heig) {
    double pixel = 0;
    for (int k = 0; k < filterMatrix->size; k++) {
        for (int l = 0; l < filterMatrix->size; l++) {
            int half = ceil(0.5 * filterMatrix->size);
            int CY = max(0, heig - half + l - 1);
            int GY = min(CY, imageMatrix->height - 1);
            int CX = max(0, wid - half + k - 1);
            int GX = min(CX, imageMatrix->width - 1);
            int processedPixel = imageMatrix->matrix[GY][GX];
            pixel += filterMatrix->matrix[k][l] * processedPixel;
        }
    }
    outputMatrix->matrix[heig][wid] = round(pixel);
}

void *blockMode(void *id) {
    long start = getCurrentTime();
    int current = (*(int *) id);
    int sector = (int) ceil(1.0 * imageMatrix->width / threadNum);
    for (int wid = current * sector; wid < (current + 1) * sector && wid < imageMatrix->width; wid++) {
        for (int heig = 0; heig < imageMatrix->height; heig++) {
            filter(wid, heig);
        }
    }

    long *tmp = malloc(sizeof(long));
    *tmp = getCurrentTime() - start;
    pthread_exit(tmp);
}

void *interleavedMode(void *id) {
    long start = getCurrentTime();
    int thisId = (*(int *) id);
    for (int wid = thisId; wid < imageMatrix->width; wid += threadNum) {
        for (int heig = 0; heig < imageMatrix->height; heig++) {
            filter(wid, heig);
        }
    }
    long *tmp = malloc(sizeof(long));
    *tmp = getCurrentTime() - start;
    pthread_exit(tmp);
}

long getCurrentTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
}

void createOutImage(FILE *outFile) {
    if (outFile == NULL)
        printError("ERROR");
    fprintf(outFile, "P2\n%i %i\n255\n", outputMatrix->width, outputMatrix->height);
    for (int y = 0; y < outputMatrix->height; y++) {
        for (int x = 0; x < outputMatrix->width; x++) {

            fprintf(outFile, "%i", outputMatrix->matrix[y][x] > 0 ? outputMatrix->matrix[y][x] : 0);
            if (x + 1 != outputMatrix->width)
                fputc(' ', outFile);
        }
        fputc('\n', outFile);
    }
    fclose(outFile);
}

void printError(char *message) {
    printf("ERROR: %s\n", message);
    exit(1);
}