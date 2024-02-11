#ifndef grid_generator_C
#define grid_generator_C

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "grid-generator.h"



int grid_generation(int rows, int cols, char* filename) {
    
    

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
            // generovani mrizek pro prvni a posledni radek a okraje
            if (i < TOTAL_SIZE_COLS + 1 || i > (TOTAL_SIZE_ROWS - 1) * (TOTAL_SIZE_COLS + 1) || i % (TOTAL_SIZE_COLS + 1) == 1 || i % (TOTAL_SIZE_COLS + 1) == TOTAL_SIZE_COLS) {
                matrix[i] = '#';
            } else {
                matrix[i] = ' ';
            } 
        }
    }

    //nutno vygenerovat aspon jeden cilovy stav uspechu a neuspechu
    int goal_number = allocated * (rand() / (RAND_MAX + 1.0));

    //fprintf(stderr, "goal_number:%d ....... %d\n", goal_number, rand());
    while (matrix[goal_number] != ' ') {
        //fprintf(stderr, "goal_number:%d", goal_number);
        goal_number = (int) allocated * (rand() /( RAND_MAX + 1.0));
    }
    matrix[goal_number] = 'G';

    int fail_number = (int) allocated * (rand() / (RAND_MAX + 1.0));
    while (matrix[fail_number] != ' ') {
        fail_number = (int) allocated * (rand() / (RAND_MAX + 1.0));
    }
    matrix[fail_number] = 'F';

    // vygenerovat prekazky, cile, neuspechy, pasti a odmeny
    // TODO pravdepodobnosti do promennych, at se daji lehce menit
    for (int i = 0; i < allocated; i++) {
        if (matrix[i] == ' ') {
            random_number = rand() / (RAND_MAX + 1.0);
            //pod 10% 
            if (random_number < 0.1) {
                matrix[i] = '#';
            //pod 12.5%
            } else if (random_number < 0.125){
                matrix[i] = 'T';
            //pod 15%
            } else if (random_number < 0.15){
                matrix[i] = 'B';
            } else if (random_number < 0.165) {
                matrix[i] = 'G';
            } else if (random_number < 0.18) {
                matrix[i] = 'F';
            }
            //jinak zustava mezera
        }
        //jinak zustava prekazka/cil/neuspech
    }



    //fprintf(stderr, "%d\n", RAND_MAX);

    matrix[allocated] = '\0';

    fprintf(file, "%s", matrix);

    free(matrix);

    fseek(file, 0, SEEK_SET);

    fclose(file);

    return 0;
}


#endif