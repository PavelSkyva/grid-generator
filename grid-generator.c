#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


//TODO predavat z prikazove radky
//pro oba rozmery definovat ruzne konstanty
//pro implementaci prostredi asi zavest z-kovou souradnici v poli a ukladat do ni priznaky prostredi
#define MATRIX_COUNT 10
#define PADDING_SIZE 1
#define AVAILABLE_STATES_COUNT 85
#define OBSTACLE -1
#define BORDER 0
#define totalSize MATRIX_COUNT+2*PADDING_SIZE
#define NUMBER_OF_OBSERVATIONS 6

int obstacles[] = {11,18,23,25,33,35,43,45,55,64,66,74,76,84,86}; // indexy prekazek, TODO -- nechat zadat z radky, asi namalovat grid
int obstacle_index = 0; // index posledni prozkoumavane prekazky
int help = 0; // pro vyrovnani preskakovani prekazek
int goals[] = {10, 85};
int failures[] = {42};
int traps[] = {14};
int bounties[] = {81};
int goals_index = 0;
int failures_index = 0;
int traps_index = 0;
int bounties_index = 0;

//TODO vypsat strukturovani cassandra souboru


void mergeAndSortArrays(int mergedArray[], int goals[], int failures[], int traps[], int bounties[], int sizes[]) {

    int size = sizes[0] + sizes[1] + sizes[2] + sizes[3];

    int *tempArray = (int *)malloc(size * sizeof(int));

    int index = 0;
    for (int i = 0; i < sizes[0]; i++) {
        tempArray[index++] = goals[i];
    }
    for (int i = 0; i < sizes[1]; i++) {
        tempArray[index++] = failures[i];
    }
    for (int i = 0; i < sizes[2]; i++) {
        tempArray[index++] = traps[i];
    }
    for (int i = 0; i < sizes[3]; i++) {
        tempArray[index++] = bounties[i];
    }

    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (tempArray[j] > tempArray[j + 1]) {
                int temp = tempArray[j];
                tempArray[j] = tempArray[j + 1];
                tempArray[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < totalSize; i++) {
        mergedArray[i] = tempArray[i];
    }


    free(tempArray);
}


void action_north(int matrix[][totalSize]) {
    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            for (int k = 1 ; k <= AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i-1][j] == OBSTACLE || matrix[i-1][j] == BORDER) {
                        if (k == matrix[i][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                printf("\n"); 
        }
    }
}


void action_south(int matrix[][totalSize]) {

    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            for (int k = 1 ; k <= AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i+1][j] == OBSTACLE || matrix[i+1][j] == BORDER) {
                        if (k == matrix[i][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                printf("\n"); 
        }
    }
}

void action_east(int matrix[][totalSize]) {

    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            for (int k = 1 ; k <= AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j+1] == OBSTACLE || matrix[i][j+1] == BORDER) {
                        if (k == matrix[i][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) 
                printf("\n"); 
        }
    }
}

void action_west(int matrix[][totalSize]) {

    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            for (int k = 1 ; k <= AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j-1] == OBSTACLE || matrix[i][j-1] == BORDER) {
                        if (k == matrix[i][j]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            printf("1.0 ");
                        } else {
                            printf("0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) 
                printf("\n"); 
        }
    }
}

void observations(int matrix[][totalSize]) {
    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index good observation (nutno poté dynamicky)
                    if (k == 4) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
                goals_index++;

            } else if (matrix[i][j] == failures[failures_index]) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index bad observation (nutno poté dynamicky)
                    if (k == 5) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
                failures_index++;
            } else if ((matrix[i][j-1] == OBSTACLE || matrix[i][j-1] == BORDER) && (matrix[i][j+1] == OBSTACLE || matrix[i][j+1] == BORDER)) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index both observation (nutno poté dynamicky)
                    if (k == 3) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
            } else if (matrix[i][j-1] == OBSTACLE || matrix[i][j-1] == BORDER) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index left observation (nutno poté dynamicky)
                    if (k == 0) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
            } else if (matrix[i][j+1] == OBSTACLE || matrix[i][j+1] == BORDER) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index right observation (nutno poté dynamicky)
                    if (k == 1) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
            } else {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index neither observation (nutno poté dynamicky)
                    if (k == 2) {
                        printf("1.0 ");
                    } else {
                        printf("0.0 ");
                    }
                }
            }
        printf("\n");
        }
    }
}

void rewards(int matrix[][totalSize]) {
    goals_index = 0;
    failures_index = 0;
    for (int i = PADDING_SIZE; i < MATRIX_COUNT + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COUNT + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                printf("R: * : %d : * : * : 1.0\n", matrix[i][j]);
                goals_index++;
            } else if (matrix[i][j] == failures[failures_index]) {
                printf("R: * : %d : * : * : -1.0\n", matrix[i][j]);
                failures_index++;
            } else {
                printf("R: * : %d : * : * : -0.04\n", matrix[i][j]);
            }
        }
    }
}




int main() {

    float discount = 0.95;
    printf("discount: %3f\n", discount);

    printf("values: reward\n");

    printf("states: %d\n", AVAILABLE_STATES_COUNT);

    //do pole retezcu
    printf("actions: n s e w\n");

    //do pole retezcu
    printf("observations: left right neither both good bad\n");

    int sizes[] = {sizeof(goals) / sizeof(goals[0]), sizeof(failures) / sizeof(failures[0]), sizeof(traps) / sizeof(traps[0]), sizeof(bounties) / sizeof(bounties[0])}; // Number of elements in each array

    int size = sizes[0] + sizes[1] + sizes[2] + sizes[3];
    int mergedArray[size];

    mergeAndSortArrays(mergedArray, goals, failures, traps, bounties, sizes);

    printf("start exclude: ");
    int marked_spot_index = 0;
    for (int i = 1; i <= AVAILABLE_STATES_COUNT; i++) {
        if (i == mergedArray[marked_spot_index]) {
            marked_spot_index++;
            printf("%d ", i);
        }
        
    }

    printf("\n");

    int matrix[totalSize][totalSize];

    for (int i = 0 ; i < AVAILABLE_STATES_COUNT; i++) {
        if (i > 7) {
            printf("-%d-", i + 1);
        } else {
            printf("-%d- ", i + 1);
        }
        
    }
    printf("\n");

    for (int i = 0 ; i < totalSize; i++) {
        for (int j = 0 ; j < totalSize ; j++) {
            matrix[i][j] = 0;
        }
    }
    
    int count = 1;

    for (int i = PADDING_SIZE; i < PADDING_SIZE + MATRIX_COUNT; i++) {
        for (int j = PADDING_SIZE; j < PADDING_SIZE + MATRIX_COUNT; j++) {
            if (count - 1 + help == obstacles[obstacle_index]) {
                matrix[i][j] = OBSTACLE;
                help++;
                obstacle_index++;
            } else {
                matrix[i][j] = count++;
            }
            
        }
    }


    /*  PRINTING of the matrix

    for (int i = 0; i < totalSize; i++) {
        for (int j = 0; j < totalSize; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }

    */

    printf("T: s\n");
    action_south(matrix);
    printf("\n\n");

    printf("T: n\n");
    action_north(matrix);
    printf("\n\n");

    printf("T: e\n");
    action_east(matrix);
    printf("\n\n");

    printf("T: w\n");
    action_west(matrix);
    printf("\n\n");

    printf("O: *\n");
    observations(matrix);
    printf("\n\n");

    rewards(matrix);


    return 0;
}
