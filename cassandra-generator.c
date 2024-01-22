#ifndef cassandra_generator_C
#define cassandra_generator_C

#include "grid-generator.h"
#include "cassandra-generator.h"



//int obstacles[] = {}; // indexy prekazek, TODO -- nechat zadat z radky, asi namalovat grid
int MATRIX_ROWS;
int MATRIX_COLS;
int TOTAL_SIZE_COLS;
int TOTAL_SIZE_ROWS;
int obstacle_index = 0; // index posledni prozkoumavane prekazky
int help = 0; // pro vyrovnani preskakovani prekazek
int *goals;
int *failures;
int *traps;
int *bounties;
int goals_index = 0;
int failures_index = 0;
int traps_index = 0;
int bounties_index = 0;
//retezec pro uchovani jmena souboru z cmd
char *matrix_arg;
int AVAILABLE_STATES_COUNT;
int border_obstacles_count;
int obstacles_count;


//TODO vypsat strukturovani cassandra souboru

int args_parse(int argc, char **argv) {
    //zpracovat argumenty
    //vymyslet prepinace
    /*
    ----co predavat: -----------------------------------------------
    obrazek matice
    # - prekazky
    G v obrazku - cile
    F v obrazku - neuspech
    T v obrazku - pasti
    B - odmeny

    --- dalsi obrazek -- zjednouduseny ------------------------------------
    * - kluzke prostredi (zmeni se smer akce)
    ! - nepruchodne prostredi (tezko se dostava do dalsich stavu)

    --- ostatni parametry--------------------------------------------------
    nazvy akci oddelene carkou (n,s,e,w) --- prepinac -actions
    discount (mensi nez 1) --- prepinac -discount   (u akci a discountu udelat implicitni, pokud nebude zadano z radky)
    mozna nahodne rozmisteni prekazek, pasti a odmen, pokud nemame specifické pozadavky
    rozmery gridu!!!!

    */

    for (int i = 1 ; i < argc; i += 2) {
        if (strcmp(argv[i], "-matrix") == 0) {
            size_t length = strlen(argv[i+1]);
            matrix_arg = malloc(length + 1);

            if (matrix_arg != NULL) {
                strncpy(matrix_arg, argv[i + 1], length);
                matrix_arg[length] = '\0';
            } else {  
                fprintf(stderr, "Error: Failed to allocate memory for matrix_arg\n");
                return 1;
            }
            
        } else if (strcmp(argv[i], "-rows") == 0) {
            MATRIX_ROWS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-cols") == 0) {
            MATRIX_COLS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Spousteni skriptu:\n\t./cassandra-generator -matrix <nazev souboru s obrazkem matice> -rows <pocet_radku> -cols <pocet_sloupcu>\n\n(Na poradi parametru nezalezi)\n");
            return 1;
        } else {
            printf("Spatne zadane parametry, zkuste --help\n");
            return 1;
        }
    }




    return 0;
}


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

    for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
        mergedArray[i] = tempArray[i];
    }

    free(tempArray);
}


void action_north(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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


void action_south(int matrix[][TOTAL_SIZE_COLS]) {

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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

void action_east(int matrix[][TOTAL_SIZE_COLS]) {

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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

void action_west(int matrix[][TOTAL_SIZE_COLS]) {

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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

void observations(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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

void rewards(int matrix[][TOTAL_SIZE_COLS]) {
    goals_index = 0;
    failures_index = 0;
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
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



long getFileSize(FILE *file) {
    long size;

    long currentPosition = ftell(file);

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    fseek(file, currentPosition, SEEK_SET);

    return size;
}


/*TODO 
    -udelat prepinac RANDOM pro zvoleni nahodne vsech hodnot a stavu, zavolat grid generation
*/

int main(int argc, char **argv) {

    if (args_parse(argc, argv)) {
        return 1;
    }

    TOTAL_SIZE_COLS = (MATRIX_COLS+2*PADDING_SIZE);
    TOTAL_SIZE_ROWS = (MATRIX_ROWS+2*PADDING_SIZE);
    border_obstacles_count = 2 * TOTAL_SIZE_COLS + 2 * TOTAL_SIZE_ROWS - 4;

    grid_generation(MATRIX_ROWS, MATRIX_COLS, matrix_arg);
    

    FILE *file = fopen(matrix_arg, "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // pro nenáhodné vygenerovani gridu
    //long fileSize = getFileSize(file);

    int matrix[TOTAL_SIZE_ROWS][TOTAL_SIZE_COLS];

    fseek(file, 0, SEEK_SET);

    for (int i = 0 ; i < TOTAL_SIZE_ROWS; i++) {
        for (int j = 0 ; j < TOTAL_SIZE_COLS ; j++) {
            if((matrix[i][j] = fgetc(file)) == '\n'){
                j--;
            }       
        }
    }

    fprintf(stderr, "\n");
    
    int state_count = 1;

    for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
        for (int j = 0; j < TOTAL_SIZE_COLS; j++) {
            if (matrix[i][j] == '#') {
                matrix[i][j] = OBSTACLE;
                obstacles_count++;
            } else {
                if (matrix[i][j] == 'G') {
                    matrix[i][j] = GOAL;
                    goals_index++;
                } else if (matrix[i][j] == 'F') {
                    matrix[i][j] = FAILURE;
                    failures_index++;
                } else if (matrix[i][j] == 'T') {
                    matrix[i][j] = TRAP;
                    traps_index++;
                } else if (matrix[i][j] == 'B') {
                    matrix[i][j] = BOUNTY;
                    bounties_index++;
                } else {
                    matrix[i][j] = state_count;
                }
                state_count++;
            }
            
        }
    }

    

    int sizes[] = {goals_index, failures_index, traps_index, bounties_index}; // Number of elements in each array

    fprintf(stderr, "number_of_goals: %d, number_of_failures: %d, number_of_traps: %d, number_of_bounties: %d\n", goals_index, failures_index, traps_index, bounties_index);

    

    //alokovani mista pro cisla stavu jednotlivych cilu, pasti....
    goals = malloc(goals_index * sizeof(int));
    failures = malloc(failures_index * sizeof(int));
    traps = malloc(traps_index * sizeof(int));
    bounties = malloc(bounties_index * sizeof(int));

    goals_index = failures_index = traps_index = bounties_index = 0;
    state_count = 1;

    
    for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
        for (int j = 0; j < TOTAL_SIZE_COLS; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == GOAL) {
                goals[goals_index] = state_count;
                fprintf(stderr, "goal: %d---", goals[goals_index]);
                goals_index++;
                matrix[i][j] = state_count++;
            } else if (matrix[i][j] == FAILURE) {
                failures[failures_index] = state_count;
                fprintf(stderr, "failure: %d---", failures[failures_index]);
                failures_index++;
                matrix[i][j] = state_count++;
            } else if (matrix[i][j] == TRAP) {
                traps[traps_index] = state_count;
                fprintf(stderr, "trap: %d---", traps[traps_index]);
                traps_index++;
                matrix[i][j] = state_count++;
            } else if (matrix[i][j] == BOUNTY) {
                bounties[bounties_index] = state_count;
                fprintf(stderr, "bounty: %d---", bounties[bounties_index]);
                bounties_index++;
                matrix[i][j] = state_count++;
            } else {
                state_count++;
            }
        }
    }

    fprintf(stderr, "\n");
    
    for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
        for (int j = 0; j < TOTAL_SIZE_COLS; j++) {
            fprintf(stderr, "%3d ", matrix[i][j]);
        }
        fprintf(stderr, "\n");
    }




    

    AVAILABLE_STATES_COUNT = (TOTAL_SIZE_ROWS * TOTAL_SIZE_COLS) - border_obstacles_count - (obstacles_count - border_obstacles_count);

    float discount = 0.95;
    printf("discount: %3f\n", discount);

    printf("values: reward\n");

    printf("states: %d\n", AVAILABLE_STATES_COUNT);

    //do pole retezcu
    printf("actions: n s e w\n");

    //do pole retezcu
    printf("observations: left right neither both good bad\n");

    



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

    free(matrix_arg);

    fclose(file);


    return 0;
}

#endif
