#ifndef drn_generator_C
#define drn_generator_C

#include "grid-generator.h"
#include "drn-generator.h"

/******************************************************
 ******************************************************
    STRANKA PRO VYTVARENI GRIDU:

        https://www.stud.fit.vutbr.cz/~xskyva04/bc/index.html


*******************************************************/


/*
    PRIKLAD SPUSTENI:

        ./cassandra_generator -matrix in.pomdp -rows 5 -cols 5 

*/



FILE *file_absorbing;

bool grid_given;

int MATRIX_ROWS;
int MATRIX_COLS;
int TOTAL_SIZE_COLS;
int TOTAL_SIZE_ROWS;
int *goals;
int *failures;
int *traps;
int *bounties;
int *starts;
int goals_index = 0;
int failures_index = 0;
int traps_index = 0;
int bounties_index = 0;
int starts_index = 0;
double random_number;
//retezec pro uchovani jmena souboru z cmd
char *input_matrix_file_string;
int AVAILABLE_STATES_COUNT;
int border_obstacles_count;
int obstacles_count;
double slip_prob;
bool slippery = false;
double impass_prob;
bool impassable = false;

//-------------------------BOOLY PRO AKCE--------------------------------------
#define NUMBER_OF_ACTIONS 8
bool north = false;
bool south = false;
bool west = false;
bool east = false;
bool northwest = false;
bool northeast = false;
bool southwest = false;
bool southeast = false;
int repeat_count;
//-----------------------------------------------------------------------------



// ------------------------HODNOTY NA UPRAVOVANI-------------------------------
double step_reward = 0;
double bounty_reward = 50.0;
double trap_reward = -10;
double goal_reward = 100.0;
double failure_reward = -100.0;
float discount = 0.95;
const char *observations_array[] = {"none", "L", "R", "U", "D", "L-R", "L-U", "L-D", "R-U", "R-D", "U-D", "L-R-U", "L-R-D", "L-U-D", "R-U-D", "all", "bad", "good"};
int observation_count = sizeof(observations_array) / sizeof(observations_array[0]);
// ----------------------------------------------------------------------------



//------------------------------------------------------------------------------


void actions_parse(char *action_string) {
    north = (action_string[0] == '1') ? true : false;
    south = (action_string[1] == '1') ? true : false;
    west = (action_string[2] == '1') ? true : false;
    east = (action_string[3] == '1') ? true : false;
    northwest = (action_string[4] == '1') ? true : false;
    northeast = (action_string[5] == '1') ? true : false;
    southwest = (action_string[6] == '1') ? true : false;
    southeast = (action_string[7] == '1') ? true : false;
}


int args_parse(int argc, char **argv) {

    bool actions_given = false;
    bool discount_given = false;
    bool samples_given = false;

    for (int i = 1 ; i < argc; i += 2) {
        

        if (strcmp(argv[i], "-matrix") == 0) {
            
            size_t length = strlen(argv[i+1]);
            input_matrix_file_string = malloc(length + 1);

            if (input_matrix_file_string != NULL) {
                strncpy(input_matrix_file_string, argv[i + 1], length);
                input_matrix_file_string[length] = '\0';
            } else {  
                fprintf(stderr, "Error: Failed to allocate memory for input_matrix_file_string\n");
                return 1;
            }
            grid_given = true;
            repeat_count = 1;
            
        } else if (strcmp(argv[i], "--impass") == 0) {
            impass_prob = strtod(argv[i+1], NULL);
            impassable = true;
        } else if (strcmp(argv[i], "--slippery") == 0) {
            slip_prob = strtod(argv[i+1], NULL);
            slippery = true;
        } else if (strcmp(argv[i], "-samples") == 0) {
            samples_given = true;
            if (grid_given) {
                printf("\nIF GRID WAS GIVEN ONLY 1 FILE WILL BE GENERATED!\n");
                repeat_count = 1;
            } else {
                repeat_count = atoi(argv[i+1]);
            }
            
        } else if (strcmp(argv[i], "-rows") == 0) {
            MATRIX_ROWS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-cols") == 0) {
            MATRIX_COLS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-actions") == 0) {
            actions_given = true;
            if (strlen(argv[i+1]) < NUMBER_OF_ACTIONS) {
                printf("prepinac -actions -------- JE TREBA ZADAT STRING O DELCE %d\n", NUMBER_OF_ACTIONS);
                return 1;
            } else {    
                actions_parse(argv[i+1]);
            }
        } else if (strcmp(argv[i], "-discount") == 0) {
            discount = strtod(argv[i+1], NULL);
            discount_given = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Spousteni skriptu:\n\t./drn_generator \n\t-matrix <nazev souboru s obrazkem matice>, pokud neni zadan, bude grid vygenerovan nahodne! \n\t-rows <pocet_radku>");
            printf("\n\t-discount <zadana hodnota discountu>");
            printf("\n\t-cols <pocet_sloupcu> \n\t--impass <pravdepodobnost, ze agent uklouzne> \n\t--slippery <pravdepodobnost, ze agent pujde kolmo k dané akci>");
            printf("\n\t-samples <pocet vygenerovanych ukazek>");
            printf("\n\t-actions xxxxxxxx (n, s , e , w, ne, nw, se, sw), 1 pokud se má akce použít, cokoli jiného a akce nebude použita (je nutno ale zadat celý string)");
            printf("\n\n(Na poradi parametru nezalezi)\n");
            
            return 1;
        } else {
            printf("Spatne zadane parametry, zkuste --help\n");
            return 1;
        }
    }
    if (!actions_given) {
        north = south = east = west = true;
    }
    if (!discount_given) {
        discount = 0.95;
    }
    if (!samples_given) {
        repeat_count = 1;
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

    for (int i = 0; i < size; i++) {
        mergedArray[i] = tempArray[i];
    }

    free(tempArray);
}

int choices() {
    int nr_of_choices = 0;

    if (north) {
        nr_of_choices++;
    }
    if (south) {
        nr_of_choices++;
    }
    if (east) {
        nr_of_choices++;
    }
    if (west) {
        nr_of_choices++;
    }


    return nr_of_choices;
}

void drn_header() {

    fprintf(file_absorbing, "@type: POMDP\n");

    fprintf(file_absorbing, "@parameters\n\n");

    fprintf(file_absorbing, "@reward_models\n");

    //muze byt vic rewardu, takze sem pridavat
    fprintf(file_absorbing, "reward\n");

    // +2, kvuli inicialnimu stavu a discount sinku
    fprintf(file_absorbing, "@nr_states\n%d\n", AVAILABLE_STATES_COUNT + 1);

    // +2, init akce a discount akce
    fprintf(file_absorbing, "@nr_choices\n%d\n", choices() * AVAILABLE_STATES_COUNT + 1);

    fprintf(file_absorbing, "@model\n");

}


void action_north_impass(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // north -- pokud je nad stavem prekazka, zustan v nem

                    if (matrix[i-1][j] == OBSTACLE) {
                        
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(file_absorbing,"\n"); 
        }
    }
}

void action_north_slip(int matrix[][TOTAL_SIZE_COLS], int i, int j) {
    fprintf(file_absorbing, "\taction n [%lf]\n", step_reward);

    float impossible_slip = 0.0;


    if (matrix[i][j-1] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j-1], slip_prob / 2);
    }

    if (matrix[i][j+1] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j+1], slip_prob / 2);
    }

    if (matrix[i-1][j] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j], 1.0 - slip_prob + impossible_slip);
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i-1][j], 1.0 - slip_prob + impossible_slip);
    }
}

void action_north(int matrix[][TOTAL_SIZE_COLS], int i, int j, int observation_number) {

    if (impassable && !(observation_number == BAD || observation_number == GOOD)) {
        action_north_impass(matrix);
        return;
    } else if (slippery && !(observation_number == BAD || observation_number == GOOD)) {
        action_north_slip(matrix, i, j);
        return;
    }
    
    fprintf(file_absorbing, "\taction n [%lf]\n", step_reward);
    
    if (observation_number == BAD || observation_number == GOOD || matrix[i-1][j] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j]);
    } else {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i-1][j]);
    }
    
}

void action_south_impass(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i+1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(file_absorbing,"\n"); 
        }
    }
}

void action_south_slip(int matrix[][TOTAL_SIZE_COLS], int i, int j) {
    fprintf(file_absorbing, "\taction s [%lf]\n", step_reward);

    float impossible_slip = 0.0;

    if (matrix[i][j-1] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j-1], slip_prob / 2);
    }

    if (matrix[i][j+1] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j+1], slip_prob / 2);
    }

    if (matrix[i+1][j] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j], 1.0 - slip_prob + impossible_slip);
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i+1][j], 1.0 - slip_prob + impossible_slip);
    }
}


void action_south(int matrix[][TOTAL_SIZE_COLS], int i, int j, int observation_number) {

    if (impassable) {
        action_south_impass(matrix);
        return;
    } else if (slippery && !(observation_number == BAD || observation_number == GOOD)) {
        action_south_slip(matrix, i, j);
        return;
    }

    fprintf(file_absorbing, "\taction s [%lf]\n", step_reward);
    
    if (observation_number == BAD || observation_number == GOOD || matrix[i+1][j] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j]);
    } else {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i+1][j]);
    }
}

void action_east_impass(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            fprintf(file_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(file_absorbing,"\n"); 
        }
    }
}

void action_east_slip(int matrix[][TOTAL_SIZE_COLS], int i, int j) {
    fprintf(file_absorbing, "\taction e [%lf]\n", step_reward);

    float impossible_slip = 0.0;

    if (matrix[i-1][j] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i-1][j], slip_prob / 2);
    }

    if (matrix[i+1][j] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i+1][j], slip_prob / 2);
    }

    if (matrix[i][j-1] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j], 1.0 - slip_prob + impossible_slip);
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j-1], 1.0 - slip_prob + impossible_slip);
    }
}

void action_east(int matrix[][TOTAL_SIZE_COLS], int i, int j, int observation_number) {

    if (impassable) {
        action_east_impass(matrix);
        return;
    } else if (slippery && !(observation_number == BAD || observation_number == GOOD)) {
        action_east_slip(matrix, i, j);
        return;
    }

    fprintf(file_absorbing, "\taction e [%lf]\n", step_reward);
    
    if (observation_number == BAD || observation_number == GOOD || matrix[i][j+1] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j]);
    } else {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j+1]);
    }
}

void action_west_impass(int matrix[][TOTAL_SIZE_COLS]) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            fprintf(file_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(file_absorbing,"\n"); 
        }
    }
}

void action_west_slip(int matrix[][TOTAL_SIZE_COLS], int i, int j) {
    fprintf(file_absorbing, "\taction w [%lf]\n", step_reward);

    float impossible_slip = 0.0;

    if (matrix[i-1][j] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i-1][j], slip_prob / 2);
    }

    if (matrix[i+1][j] == OBSTACLE) {
        impossible_slip += slip_prob / 2;
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i+1][j], slip_prob / 2);
    }

    if (matrix[i][j+1] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j], 1.0 - slip_prob + impossible_slip);
    } else {
        fprintf(file_absorbing, "\t\t%d : %lf\n", matrix[i][j+1], 1.0 - slip_prob + impossible_slip);
    }
}

void action_west(int matrix[][TOTAL_SIZE_COLS], int i, int j, int observation_number) {

    if (impassable) {
        action_west_impass(matrix);
        return;
    } else if (slippery && !(observation_number == BAD || observation_number == GOOD)) {
        action_west_slip(matrix, i, j);
        return;
    }

    fprintf(file_absorbing, "\taction w [%lf]\n", step_reward);
    
    if (observation_number == BAD || observation_number == GOOD || matrix[i][j-1] == OBSTACLE) {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j]);
    } else {
        fprintf(file_absorbing, "\t\t%d : 1.0\n", matrix[i][j-1]);
    }
}

void action_northwest(int matrix[][TOTAL_SIZE_COLS]) {


    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i-1][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);

                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i-1][j-1]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(file_absorbing,"\n"); 
                //fflush(NULL);
            }
                
        }
    }
}

void action_northeast(int matrix[][TOTAL_SIZE_COLS]) {

    /*
    if (impassable) {
        action_west_impass(matrix);
        return;
    } else if (slippery) {
        action_west_slip(matrix);
        return;
    }
    */

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i-1][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);

                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i-1][j+1]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(file_absorbing,"\n"); 
                //fflush(NULL);
            }
                
        }
    }
}

void action_southeast(int matrix[][TOTAL_SIZE_COLS]) {

    /*
    if (impassable) {
        action_west_impass(matrix);
        return;
    } else if (slippery) {
        action_west_slip(matrix);
        return;
    }
    */

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i+1][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);

                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i+1][j+1]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(file_absorbing,"\n"); 
                //fflush(NULL);
            }
                
        }
    }
}

void action_southwest(int matrix[][TOTAL_SIZE_COLS]) {

    /*
    if (impassable) {
        action_west_impass(matrix);
        return;
    } else if (slippery) {
        action_west_slip(matrix);
        return;
    }
    */

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i+1][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);

                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i+1][j-1]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(file_absorbing,"\n"); 
                //fflush(NULL);
            }
                
        }
    }
}

int observations(int matrix[][TOTAL_SIZE_COLS], int i, int j) {

    //printf("nr_of_g:%d\n", goals_index);

    for (int k = 0; k < goals_index; k++) {
        if (matrix[i][j] == goals[k]) {
            return GOOD;
        }
    }

    for (int k = 0; k < failures_index; k++) {
        if (matrix[i][j] == failures[k]) {
            return BAD;
        }
    }

    for (int k = 0; k < bounties_index; k++) {
        if (matrix[i][j] == bounties[k]) {
            return TREASURE;
        }
    }

    for (int k = 0; k < traps_index; k++) {
        if (matrix[i][j] == traps[k]) {
            return CURSE;
        }
    }


    // binárně    left|right|up|down

    int obstacles = 0;

    //nalevo
    if (matrix[i][j-1] == OBSTACLE) {
        obstacles += 8;
    }
    //napravo
    if (matrix[i][j+1] == OBSTACLE) {
        obstacles += 4;
    }
    //nahore
    if (matrix[i-1][j] == OBSTACLE) {
        obstacles += 2;
    }
    //dole
    if (matrix[i+1][j] == OBSTACLE) {
        obstacles += 1;
    }

    return obstacles;

}


void init_state() {
    fprintf(file_absorbing, "state 0 {%d} [0] init\n", START_OBS);
    fprintf(file_absorbing, "\taction init [0]\n");
    for (int i = 0; i < starts_index; i++ ) {
        fprintf(file_absorbing, "\t\t%d : %lf\n", starts[i], 1.0 / starts_index);
    }
}

/*
void discount_sink() {
    fprintf(file_absorbing, "state %d {%d} [0] discount_sink\n", AVAILABLE_STATES_COUNT + 1, START_OBS + 1);
    fprintf(file_absorbing, "\taction discount_sink [0]\n");
    fprintf(file_absorbing, "\t\t%d : 1\n", AVAILABLE_STATES_COUNT + 1);
} 
*/

int main(int argc, char **argv) {

    srand((unsigned int)time(NULL));

    if (args_parse(argc, argv)) {
        return 1;
    }

    if (input_matrix_file_string == NULL) {
        input_matrix_file_string = (char *) malloc(50);
    }

    TOTAL_SIZE_COLS = (MATRIX_COLS+2*PADDING_SIZE);
    TOTAL_SIZE_ROWS = (MATRIX_ROWS+2*PADDING_SIZE);
    border_obstacles_count = 2 * TOTAL_SIZE_COLS + 2 * TOTAL_SIZE_ROWS - 4;

    char *directory = (char *) malloc(50);


    sprintf(directory, "outputs%dx%d", MATRIX_ROWS, MATRIX_COLS);

    /*
    if (slippery) {
        strcat(directory, "_slippery");
    } else if (impassable) {
        strcat(directory, "_impassable");
    }
    */
    
    if (mkdir(directory, 0755) == -1) {
        if (errno != EEXIST) {
            perror("Error creating directory");
            return 1;
        }
    }
    
    chdir(directory);

    int repeat_number = 1;
    char *abs_file_number = (char*) malloc(50);


    
    while (repeat_number <= repeat_count) {

        sprintf(directory, "output%d/", repeat_number);
        if (mkdir(directory, 0755) == -1) {
            if (errno != EEXIST) {
                perror("Error creating directory");
                return 1;
            }
        }

        chdir(directory);

        char current_directory[1024];

        if (!grid_given) {
            
            sprintf(input_matrix_file_string, "in.drn");
            if (grid_generation(MATRIX_ROWS, MATRIX_COLS, input_matrix_file_string)) {
                printf("an error occured in generating\n");
                return 1;
            }
        } else {
            char current_directory[1024];
            if (getcwd(current_directory, sizeof(current_directory)) == NULL) {
                perror("getcwd() error");
                return 1;
            }
            chdir("..");
            chdir("..");
        }

        
        sprintf(abs_file_number, "output.drn");
    
        FILE *input_matrix_file = fopen(input_matrix_file_string, "r+");
        

        if (input_matrix_file == NULL) {
            perror("Error opening file");
            return 1;
        }

        chdir(current_directory);

        int matrix[TOTAL_SIZE_ROWS][TOTAL_SIZE_COLS];

        fseek(input_matrix_file, 0, SEEK_SET);


        //pridat kontroly symbolu (validni jsou zatim #,  , F, G, T, B)
        for (int i = 0 ; i < TOTAL_SIZE_ROWS; i++) {
            for (int j = 0 ; j < TOTAL_SIZE_COLS ; j++) {
                //osetreni odradkovani (pri nahodnem generovani je na zacatku odradkovani)
                if((matrix[i][j] = fgetc(input_matrix_file)) == '\n'){
                    j--;
                }
            }
        }
        
        
        int state_count = 1;

        //nahrazeni znaku za cisla stavu (normalnich/specialnich) + napocitani jednotlivych specialnich stavu
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
                    } else if (matrix[i][j] == 'S') {
                        matrix[i][j] = START;
                        starts_index++;
                    } else {
                        matrix[i][j] = state_count;
                    }
                    state_count++;
                }
                
            }
        }

        //indexy nam zde znaci pocet polozek v danem poli, tedy pocet cilu, pasti...

        //alokovani mista pro cisla stavu jednotlivych cilu, pasti....
        goals = malloc(goals_index * sizeof(int));
        failures = malloc(failures_index * sizeof(int));
        traps = malloc(traps_index * sizeof(int));
        bounties = malloc(bounties_index * sizeof(int));
        starts = malloc(starts_index * sizeof(int));

        if (goals == NULL || failures == NULL || bounties == NULL || traps == NULL || starts == NULL) {
            printf("chyba v alokaci\n");
            exit(0);
        }

        goals_index = failures_index = traps_index = bounties_index = starts_index = 0;
        state_count = 1;

        //nahrazeni specialnich stavu v matici
        for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
            for (int j = 0; j < TOTAL_SIZE_COLS; j++) {
                if (matrix[i][j] == OBSTACLE) {
                    continue;
                } else if (matrix[i][j] == GOAL) {
                    goals[goals_index] = state_count;
                    //fprintf(stderr, "goal: %d---", goals[goals_index]);
                    goals_index++;
                    matrix[i][j] = state_count++;
                } else if (matrix[i][j] == FAILURE) {
                    failures[failures_index] = state_count;
                    //fprintf(stderr, "failure: %d---", failures[failures_index]);
                    failures_index++;
                    matrix[i][j] = state_count++;
                } else if (matrix[i][j] == TRAP) {
                    traps[traps_index] = state_count;
                    //fprintf(stderr, "trap: %d---", traps[traps_index]);
                    traps_index++;
                    matrix[i][j] = state_count++;
                } else if (matrix[i][j] == BOUNTY) {
                    bounties[bounties_index] = state_count;
                    //fprintf(stderr, "bounty: %d---", bounties[bounties_index]);
                    bounties_index++;
                    matrix[i][j] = state_count++;
                } else if (matrix[i][j] == START) {
                    starts[starts_index] = state_count;
                    starts_index++;
                    matrix[i][j] = state_count++;
                } else {
                    state_count++;
                }
            }
        }
        
        fprintf(input_matrix_file, "\n\n");
        
        //printeni matice do souboru in<cislo>.pomdp
        for (int i = 0; i < TOTAL_SIZE_ROWS; i++) {
            for (int j = 0; j < TOTAL_SIZE_COLS; j++) {
                fprintf(input_matrix_file, "%3d ", matrix[i][j]);
            }
            fprintf(input_matrix_file, "\n");
        }
        
        

        AVAILABLE_STATES_COUNT = (TOTAL_SIZE_ROWS * TOTAL_SIZE_COLS) - border_obstacles_count - (obstacles_count - border_obstacles_count);

        
        file_absorbing = fopen(abs_file_number, "w+");

        if (file_absorbing == NULL) {
            perror("Error opening file");
            return 1;
        } 

        drn_header();

        init_state();

        int observation_number;
        //vypisovani jednotlivych stavu + jejich akci
        for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
            for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
                if (matrix[i][j] == OBSTACLE) {
                    continue;
                }
                fprintf(file_absorbing, "state %d ", matrix[i][j]);
                observation_number = observations(matrix, i, j);
                if (observation_number == BAD) {
                    fprintf(file_absorbing, "{%d} [%f] fail\n", observation_number, failure_reward);
                } else if (observation_number == GOOD) {
                    fprintf(file_absorbing, "{%d} [%f] goal\n", observation_number, goal_reward);
                } else if (observation_number == TREASURE) {
                    fprintf(file_absorbing, "{%d} [%f] treasure\n", observation_number, bounty_reward);
                } else if (observation_number == CURSE) {
                    fprintf(file_absorbing, "{%d} [%f] curse\n", observation_number, trap_reward);
                } else {
                    fprintf(file_absorbing, "{%d} [0]\n", observation_number);
                }
                
                if (north) {
                    action_north(matrix, i, j, observation_number);
                }
                if (south) {
                    action_south(matrix, i, j, observation_number);
                }
                if (east) {
                    action_east(matrix, i, j, observation_number);
                }
                if (west) {
                    action_west(matrix, i, j, observation_number);
                }
            }
        }

        
        //discount_sink();

        


        free(goals);
        free(traps);
        free(bounties);
        free(failures);


        fclose(file_absorbing);

        fclose(input_matrix_file);
        chdir("..");
        repeat_number++;
        //nutno resetovat countery
        goals_index = failures_index = traps_index = bounties_index = obstacles_count = 0;
    }

    free(abs_file_number);
    free(input_matrix_file_string);
    free(directory);

    return 0;
}

#endif
