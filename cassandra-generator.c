#ifndef cassandra_generator_C
#define cassandra_generator_C

#include "grid-generator.h"
#include "cassandra-generator.h"

/******************************************************
 ******************************************************
    STRANKA PRO VYTVARENI GRIDU:

        https://www.stud.fit.vutbr.cz/~xskyva04/bc/index.html


*******************************************************/


/*
    PRIKLAD SPUSTENI:

        ./cassandra_generator -matrix in.pomdp -rows 5 -cols 5 -discount 0.95 

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
int goals_index = 0;
int failures_index = 0;
int traps_index = 0;
int bounties_index = 0;
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
double step_reward = -0.04;
double bounty_reward = 10;
double trap_reward = -10;
double goal_reward = 1.0;
double failure_reward = -1.0;
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
            printf("Spousteni skriptu:\n\t./cassandra_generator \n\t-matrix <nazev souboru s obrazkem matice>, pokud neni zadan, bude grid vygenerovan nahodne! \n\t-rows <pocet_radku>");
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

void cassandra_header() {
            
    fprintf(file_absorbing, "discount: %3f\n", discount);
    //fflush(NULL);

    fprintf(file_absorbing,"values: reward\n");
    //fflush(NULL);

    fprintf(file_absorbing,"states: %d\n", AVAILABLE_STATES_COUNT);
    //fflush(NULL);

    //do pole retezcu
    fprintf(file_absorbing, "actions:");
    if (north) {
        fprintf(file_absorbing, " n");
    }
    if (south) {
        fprintf(file_absorbing, " s");
    }
    if (east) {
        fprintf(file_absorbing, " e");
    }
    if (west) {
        fprintf(file_absorbing, " w");
    }
    if (northeast) {
        fprintf(file_absorbing, " ne");
    }
    if (northwest) {
        fprintf(file_absorbing, " nw");
    }
    if (southeast) {
        fprintf(file_absorbing, " se");
    }
    if (southwest) {
        fprintf(file_absorbing, " sw");
    }
    fprintf(file_absorbing, "\n");

    fprintf(file_absorbing, "observations: ");

    for (int i = 0; i < observation_count; i++)
    {
        fprintf(file_absorbing, "%s ", observations_array[i]);
    }
    
    fprintf(file_absorbing, "\n");

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

void action_north_slip(int matrix[][TOTAL_SIZE_COLS]) {
    //pro vyrovavani, pokud na strane, kam agent uklouzl, je prekazka
    double slip_help = 0.0;
    // booly, aby se zabranilo tisknuti pri prekazkach
    bool do_not_print_l = false;
    bool do_not_print_r = false;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //zjisteni zda do na potencialne uklouznutelne strany jsou prekazky
            if (matrix[i][j+1] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_r = true;
            }
            if (matrix[i][j-1] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_l = true;
            }
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // north -- pokud je nad stavem prekazka, zustan v nem, ale mohl uklouznout 
                    if (matrix[i-1][j] == OBSTACLE) {
                        if ((k == matrix[i][j+1] && !do_not_print_r) || (k == matrix[i][j-1] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i][j+1] && !do_not_print_r) || (k == matrix[i][j-1] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
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
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_north(int matrix[][TOTAL_SIZE_COLS]) {

    if (impassable) {
        action_north_impass(matrix);
        return;
    } else if (slippery) {
        action_north_slip(matrix);
        return;
    }
    

    
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i-1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(file_absorbing,"1.0 ");
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

void action_south_slip(int matrix[][TOTAL_SIZE_COLS]) {
    //pro vyrovavani, pokud na strane, kam agent uklouzl, je prekazka
    double slip_help = 0.0;
    //booly, aby se zabranilo tisknuti pri prekazkach
    bool do_not_print_l = false;
    bool do_not_print_r = false;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //zjisteni zda do na potencialne uklouznutelne strany jsou prekazky
            if (matrix[i][j-1] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_r = true;
            }
            if (matrix[i][j+1] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_l = true;
            }
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // south -- pokud pod stavem prekazka, zustan v nem, ale mohl uklouznout 
                    if (matrix[i+1][j] == OBSTACLE) {
                        if ((k == matrix[i][j-1] && !do_not_print_r) || (k == matrix[i][j+1] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i][j-1] && !do_not_print_r) || (k == matrix[i][j+1] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
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
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}


void action_south(int matrix[][TOTAL_SIZE_COLS]) {

    if (impassable) {
        action_south_impass(matrix);
        return;
    } else if (slippery) {
        action_south_slip(matrix);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i+1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
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
            if (matrix[i][j] != OBSTACLE) {
                fprintf(file_absorbing,"\n"); 
                //fflush(NULL);
            }
                
        }
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

void action_east_slip(int matrix[][TOTAL_SIZE_COLS]) {
    //pro vyrovavani, pokud na strane, kam agent uklouzl, je prekazka
    double slip_help = 0.0;
    // booly, aby se zabranilo tisknuti pri prekazkach
    bool do_not_print_l = false;
    bool do_not_print_r = false;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //zjisteni zda do na potencialne uklouznutelne strany jsou prekazky
            if (matrix[i+1][j] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_r = true;
            }
            if (matrix[i-1][j] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_l = true;
            }
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // south -- pokud pod stavem prekazka, zustan v nem, ale mohl uklouznout 
                    if (matrix[i][j+1] == OBSTACLE) {
                        if ((k == matrix[i+1][j] && !do_not_print_r) || (k == matrix[i-1][j] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i+1][j] && !do_not_print_r) || (k == matrix[i-1][j] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
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
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_east(int matrix[][TOTAL_SIZE_COLS]) {

    if (impassable) {
        action_east_impass(matrix);
        return;
    } else if (slippery) {
        action_east_slip(matrix);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
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

void action_west_slip(int matrix[][TOTAL_SIZE_COLS]) {
    //pro vyrovavani, pokud na strane, kam agent uklouzl, je prekazka
    double slip_help = 0.0;
    // booly, aby se zabranilo tisknuti pri prekazkach
    bool do_not_print_l = false;
    bool do_not_print_r = false;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //zjisteni zda do na potencialne uklouznutelne strany jsou prekazky
            if (matrix[i-1][j] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_r = true;
            }
            if (matrix[i+1][j] == OBSTACLE) {
                slip_help += (slip_prob / 2);
                do_not_print_l = true;
            }
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // south -- pokud pod stavem prekazka, zustan v nem, ale mohl uklouznout 
                    if (matrix[i][j-1] == OBSTACLE) {
                        if ((k == matrix[i-1][j] && !do_not_print_r) || (k == matrix[i+1][j] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(file_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            fprintf(file_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i-1][j] && !do_not_print_r) || (k == matrix[i+1][j] && !do_not_print_l)) {
                            fprintf(file_absorbing,"%f ", slip_prob / 2);
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
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_west(int matrix[][TOTAL_SIZE_COLS]) {

    if (impassable) {
        action_west_impass(matrix);
        return;
    } else if (slippery) {
        action_west_slip(matrix);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(file_absorbing,"1.0 ");
                            //fflush(NULL);

                        } else {
                            fprintf(file_absorbing,"0.0 ");
                            //fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
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

void action_northwest(int matrix[][TOTAL_SIZE_COLS]) {

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

void observations(int matrix[][TOTAL_SIZE_COLS]) {
    goals_index = failures_index = traps_index = bounties_index = 0;
    fprintf(file_absorbing, "# 0none 1L 2R 3U 4D 5L-R 6L-U 7L-D 8R-U 9R-D 10U-D 11L-R-U 12L-R-D 13L-U-D 14R-U-D 15all 16bad 17good\n");

    fprintf(file_absorbing,"O: *\n");
    fprintf(file_absorbing,"#0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17\n");
    
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                for (int k = 0; k < observation_count; k++) {
                    //index good observation
                    //strcasecmp(observations_array[k], "good")
                    if (k == 17) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
                goals_index++;

            } else if (matrix[i][j] == failures[failures_index]) {
                for (int k = 0; k < observation_count; k++) {
                    //index bad observation
                    //strcasecmp(observations_array[k], "bad")
                    if (k == 16) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
                failures_index++;

            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i][j+1] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index all observation
                    //strcasecmp(observations_array[k], "all")
                    if (k == 15) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j+1] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index R-U-D (right-up-down)
                    //strcasecmp(observations_array[k], "R-U-D"), obdobne u vsech dalsich
                    if (k == 14) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-U-D (left-up-down)
                    if (k == 13) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i][j+1] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-R-D
                    if (k == 12) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i][j+1] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-R-U
                    if (k == 11) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i-1][j] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index U-D
                    if (k == 10) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j+1] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index R-D
                    if (k == 9) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j+1] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index R-U
                    if (k == 8) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i+1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-D
                    if (k == 7) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i-1][j] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-U
                    if (k == 6) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i][j+1] == OBSTACLE)) {
                for (int k = 0; k < observation_count; k++) {
                    //index L-R
                    if (k == 5) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if (matrix[i+1][j] == OBSTACLE) {
                for (int k = 0; k < observation_count; k++) {
                    //index D
                    if (k == 4) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if (matrix[i-1][j] == OBSTACLE) {
                for (int k = 0; k < observation_count; k++) {
                    //index U
                    if (k == 3) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if (matrix[i][j+1] == OBSTACLE) {
                for (int k = 0; k < observation_count; k++) {
                    //index R
                    if (k == 2) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else if (matrix[i][j-1] == OBSTACLE) {
                for (int k = 0; k < observation_count; k++) {
                    //index L
                    if (k == 1) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            } else {
                for (int k = 0; k < observation_count; k++) {
                    //index none
                    if (k == 0) {
                        fprintf(file_absorbing,"1.0 ");
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                    }
                }
            }
        fprintf(file_absorbing,"\n");
        }
    }
}

void rewards(int matrix[][TOTAL_SIZE_COLS]) {
    goals_index = failures_index = traps_index = bounties_index = 0;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                fprintf(file_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], goal_reward);
                //fflush(NULL);
                goals_index++;
            } else if (matrix[i][j] == failures[failures_index]) {
                fprintf(file_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], failure_reward);
                //fflush(NULL);
                failures_index++;
            } else if(matrix[i][j] == traps[traps_index]) {
                fprintf(file_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], trap_reward); 
                //fflush(NULL);
                traps_index++;
            } else if (matrix[i][j] == bounties[bounties_index]) {
                fprintf(file_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], bounty_reward);
                //fflush(NULL);
                bounties_index++;
            } else {
                fprintf(file_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], step_reward);
                //fflush(NULL);
            }
        }
    }
}

void generate_exceptions(int matrix[][TOTAL_SIZE_COLS]) {
    goals_index = failures_index = 0;
    
    fprintf(file_absorbing,"\n");
    //fflush(NULL);
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            
            if (matrix[i][j] == goals[goals_index]) {

                fprintf(file_absorbing,"T: * : %d\n", matrix[i][j]);
                //fflush(NULL);
                for (int k = 0; k < AVAILABLE_STATES_COUNT; k++) {
                    if (k == matrix[i][j]) {
                        fprintf(file_absorbing,"1.0 ");
                        //fflush(NULL);
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                        //fflush(NULL);
                    }
                }
                
                fprintf(file_absorbing,"\n\n");
                //fflush(NULL);
                goals_index++;
            } else if (matrix[i][j] == failures[failures_index]) {
                fprintf(file_absorbing,"T: * : %d\n", matrix[i][j]);
                //fflush(NULL);
                
                for (int k = 0; k < AVAILABLE_STATES_COUNT; k++) {
                    if (k == matrix[i][j]) {
                        fprintf(file_absorbing,"1.0 ");
                        //fflush(NULL);
                    } else {
                        fprintf(file_absorbing,"0.0 ");
                        //fflush(NULL);
                    }
                }
                
                fprintf(file_absorbing,"\n\n");
                //fflush(NULL);
                failures_index++;
            }
        }
    }
}


/*
long getFileSize(FILE *file) {
    long size;

    long currentPosition = ftell(file);

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    fseek(file, currentPosition, SEEK_SET);

    return size;
}


void matrix_print(int rows, int cols) {

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

    if (slippery) {
        strcat(directory, "_slippery");
    } else if (impassable) {
        strcat(directory, "_impassable");
    }

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
            
            sprintf(input_matrix_file_string, "in.pomdp");
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

        
        sprintf(abs_file_number, "absorbing.pomdp");

        
    
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
        
        int state_count = 0;

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
                    } else {
                        matrix[i][j] = state_count;
                    }
                    state_count++;
                }
                
            }
        }
        //indexy nam zde znaci pocet polozek v danem poli, tedy pocet cilu, pasti...
        // promenna pro pocty jednotlivych specialnich stavu, pouzito ve fci mergeAndSortArrays
        int sizes[] = {goals_index, failures_index, traps_index, bounties_index};


        //alokovani mista pro cisla stavu jednotlivych cilu, pasti....
        goals = malloc(goals_index * sizeof(int));
        failures = malloc(failures_index * sizeof(int));
        traps = malloc(traps_index * sizeof(int));
        bounties = malloc(bounties_index * sizeof(int));

        goals_index = failures_index = traps_index = bounties_index = 0;
        state_count = 0;

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
                } else {
                    state_count++;
                }
            }
        }

        fprintf(input_matrix_file, "\n");
        
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

        cassandra_header();
            

        int size = sizes[0] + sizes[1] + sizes[2] + sizes[3];
        int mergedArray[size];

            

        mergeAndSortArrays(mergedArray, goals, failures, traps, bounties, sizes);


        fprintf(file_absorbing,"start exclude: ");
        //fflush(NULL);
        int marked_spot_index = 0;
        for (int i = 0; i < AVAILABLE_STATES_COUNT; i++) {
            if (i == mergedArray[marked_spot_index]) {
                marked_spot_index++;
                fprintf(file_absorbing,"%d ", i);
            }
            
        }     

        fprintf(file_absorbing,"\n");
        //fflush(NULL);

        if (north) {
            fprintf(file_absorbing,"T: n\n");
            //fflush(NULL);
            action_north(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }
        
        if (south) {
            fprintf(file_absorbing,"T: s\n");
            //fflush(NULL);
            action_south(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL); 
        }    

        if (east) {
            fprintf(file_absorbing,"T: e\n");
            //fflush(NULL);
            action_east(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        if (west) {
            fprintf(file_absorbing,"T: w\n");
            //fflush(NULL);
            action_west(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        if (northwest) {
            fprintf(file_absorbing,"T: nw\n");
            //fflush(NULL);
            action_northwest(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        if (northeast) {
            fprintf(file_absorbing,"T: ns\n");
            //fflush(NULL);
            action_northeast(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        if (southwest) {
            fprintf(file_absorbing,"T: sw\n");
            //fflush(NULL);
            action_southwest(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        if (southeast) {
            fprintf(file_absorbing,"T: se\n");
            //fflush(NULL);
            action_southeast(matrix);
            fprintf(file_absorbing,"\n\n");
            //fflush(NULL);
        }

        

        generate_exceptions(matrix);


        observations(matrix);
        fprintf(file_absorbing,"\n\n");

        rewards(matrix);



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
