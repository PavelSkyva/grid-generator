#ifndef grid_generator_C
#define grid_generator_C

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "grid-generator.h"



int grid_generation(int rows, int cols, char* filename) {
    
    srand((unsigned int)time(NULL));

    double random_number;

    FILE *file = fopen(filename, "w+");

    if (file == NULL) {
        perror("Error opening file");
        fclose(file);
        return 1;
    }

    // 2 == 2 * PADDING
    // + 1 je pro vlozeni odradkovani
    int TOTAL_SIZE_COLS = cols + 2 * PADDING;
    int TOTAL_SIZE_ROWS = rows + 2 * PADDING;
    int allocated = (TOTAL_SIZE_ROWS) * (TOTAL_SIZE_COLS + 1);
    
    // pridat odradkovani => pricist ROWS
    char *matrix = (char*) malloc(allocated + 1);

    for (int i = 0 ; i < allocated; i++) {
        if (i % (TOTAL_SIZE_COLS + 1) == 0) {
            matrix[i] = '\n';
        } else {
            // generovani mrizek pro prvni a posledni radek
            if (i < TOTAL_SIZE_COLS + 1 || i > (TOTAL_SIZE_ROWS - 1) * (TOTAL_SIZE_COLS + 1) || i % (TOTAL_SIZE_COLS + 1) == 1 || i % (TOTAL_SIZE_COLS + 1) == TOTAL_SIZE_COLS) {
                matrix[i] = '#';
            } else {
                matrix[i] = ' ';
            } 
        }
    }

    for (int i = 0; i < allocated; i++) {
        if (matrix[i] == ' ') {
            random_number = rand();
            // 1/10 sance ze vygeneruje prekazku
            if (random_number < RAND_MAX / 10) {
                matrix[i] = '#';
            } else {
                matrix[i] = ' ';
            }
        }
    }

    matrix[allocated] = '\0';

    fprintf(file, "%s", matrix);

    free(matrix);

    fseek(file, 0, SEEK_SET);

    fclose(file);

    return 0;
}


#endif