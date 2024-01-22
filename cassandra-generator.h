#ifndef cassandra_generator_H
#define cassandra_generator_H


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "grid-generator.h"



//TODO predavat z prikazove radky
//pro oba rozmery definovat ruzne konstanty
//pro implementaci prostredi asi zavest z-kovou souradnici v poli a ukladat do ni priznaky prostredi
//parametry cmd


#define PADDING_SIZE 1

#define OBSTACLE -1
#define BORDER 0
#define NUMBER_OF_OBSERVATIONS 6

int args_parse(int argc, char **argv);

void mergeAndSortArrays(int mergedArray[], int goals[], int failures[], int traps[], int bounties[], int sizes[]);

long getFileSize(FILE *file);


#endif