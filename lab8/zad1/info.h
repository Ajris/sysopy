//
// Created by ajris on 19.05.19.
//

#ifndef LAB8_ZAD1_INFO_H
#define LAB8_ZAD1_INFO_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

typedef struct Filter{
    int size;
    double** matrix;
} Filter;

typedef struct Image{
    int width;
    int height;
    int** matrix;
} Image;

#endif //LAB8_ZAD1_INFO_H
