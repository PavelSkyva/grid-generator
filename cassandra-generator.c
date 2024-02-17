#ifndef cassandra_generator_C
#define cassandra_generator_C

#include "grid-generator.h"
#include "cassandra-generator.h"


/*TODO
-- usporadat konstanty do jednoho místa
-- udelat vice souboru naraz (dat konstanty, ktere to budou ovladat)
-- uchovavat i inputy => upravit tak aby se skladaly do slozky
*/
FILE *file_absorbing;
FILE *file_not_absorbing;


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
int special_states_count;
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



// ------------------------HODNOTY NA UPRAVOVANI-------------------------------
double step_reward = -0.04;
double bounty_reward = 10;
double trap_reward = -10;
double goal_reward = 1.0;
double failure_reward = -1.0;
float discount = 0.95;
// kolik bludisti vytvorit pri nahodnem generovani
int repeat_count;
// ----------------------------------------------------------------------------


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
            /*
            size_t length = strlen(argv[i+1]);
            input_matrix_file_string = malloc(length + 1);

            if (input_matrix_file_string != NULL) {
                strncpy(input_matrix_file_string, argv[i + 1], length);
                input_matrix_file_string[length] = '\0';
            } else {  
                //fprintf(stderr, "Error: Failed to allocate memory for input_matrix_file_string\n");
                return 1;
            }
            */
        } else if (strcmp(argv[i], "--impass") == 0) {
            impass_prob = strtod(argv[i+1], NULL);
            impassable = true;
        } else if (strcmp(argv[i], "--slippery") == 0) {
            slip_prob = strtod(argv[i+1], NULL);
            slippery = true;
        } else if (strcmp(argv[i], "-samples") == 0) {
            repeat_count = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-rows") == 0) {
            MATRIX_ROWS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-cols") == 0) {
            MATRIX_COLS = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Spousteni skriptu:\n\t./cassandra-generator -matrix <nazev souboru s obrazkem matice> -rows <pocet_radku> -cols <pocet_sloupcu>\n --impass <pravdepodobnost, ze agent uklouzne> --slippery <pravdepodobnost, ze agent pujde kolmo k dané akci>\n\n(Na poradi parametru nezalezi)\n");
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




void action_north_impass(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            //<POCET_DOSTUPNYCH_STAVU> x tisknuti 
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                // pokud je prekazka => preskoc
                if (matrix[i][j] != OBSTACLE) {
                    // north -- pokud je nad stavem prekazka, zustan v nem 
                    if (matrix[i-1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
        }
    }
}

void action_north_slip(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
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
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i][j+1] && !do_not_print_r) || (k == matrix[i][j-1] && !do_not_print_l)) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_north(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {

    if (impassable) {
        action_north_impass(matrix, absorbing);
        return;
    } else if (slippery) {
        action_north_slip(matrix, absorbing);
        return;
    }
    
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i-1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i-1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
        }
    }
    
}

void action_south_impass(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i+1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
        }
    }
}

void action_south_slip(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    //pro vyrovavani, pokud na strane, kam agent uklouzl, je prekazka
    double slip_help = 0.0;
    // booly, aby se zabranilo tisknuti pri prekazkach
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
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i][j-1] && !do_not_print_r) || (k == matrix[i][j+1] && !do_not_print_l)) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}


void action_south(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {

    if (impassable) {
        action_south_impass(matrix, absorbing);
        return;
    } else if (slippery) {
        action_south_slip(matrix, absorbing);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i+1][j] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i+1][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
                fflush(NULL);
            }
                
        }
    }
}

void action_east_impass(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
        }
    }
}

void action_east_slip(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
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
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i+1][j] && !do_not_print_r) || (k == matrix[i-1][j] && !do_not_print_l)) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_east(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {

    if (impassable) {
        action_east_impass(matrix, absorbing);
        return;
    } else if (slippery) {
        action_east_slip(matrix, absorbing);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j+1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i][j+1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
                fflush(NULL);
            }
                
        }
    }
}

void action_west_impass(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) { 
                    if (matrix[i][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - impass_prob);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", impass_prob);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
        }
    }
}

void action_west_slip(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
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
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", 1.0 - slip_prob + slip_help);
                        } else if ((k == matrix[i-1][j] && !do_not_print_r) || (k == matrix[i+1][j] && !do_not_print_l)) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", slip_prob / 2);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            if (matrix[i][j] != OBSTACLE) 
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
            slip_help = 0.0;
            do_not_print_l = false;
            do_not_print_r = false;
        }
    }
}

void action_west(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {

    if (impassable) {
        action_west_impass(matrix, absorbing);
        return;
    } else if (slippery) {
        action_west_slip(matrix, absorbing);
        return;
    }

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            for (int k = 0 ; k < AVAILABLE_STATES_COUNT ; k++) {
                if (matrix[i][j] != OBSTACLE) {
                    if (matrix[i][j-1] == OBSTACLE) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);

                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    } else {
                        if (k == matrix[i][j-1]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    }
                    
                } else if (matrix[i][j] == OBSTACLE) {
                    break;
                }
            }
            //aby nebyly mezi transition maticema mezery
            if (matrix[i][j] != OBSTACLE) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n"); 
                fflush(NULL);
            }
                
        }
    }
}

void observations(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index good observation (nutno poté dynamicky)
                    if (k == 4) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
                goals_index++;

            } else if (matrix[i][j] == failures[failures_index]) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index bad observation (nutno poté dynamicky)
                    if (k == 5) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
                failures_index++;
            } else if ((matrix[i][j-1] == OBSTACLE) && (matrix[i][j+1] == OBSTACLE)) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index both observation (nutno poté dynamicky)
                    if (k == 3) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
            } else if (matrix[i][j-1] == OBSTACLE) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index left observation (nutno poté dynamicky)
                    if (k == 0) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
            } else if (matrix[i][j+1] == OBSTACLE) {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index right observation (nutno poté dynamicky)
                    if (k == 1) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
            } else {
                for (int k = 0; k < NUMBER_OF_OBSERVATIONS; k++) {
                    //index neither observation (nutno poté dynamicky)
                    if (k == 2) {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                        fflush(NULL);
                    } else {
                        fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                        fflush(NULL);
                    }
                }
            }
        fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
        fflush(NULL);
        }
    }
}

void rewards(int matrix[][TOTAL_SIZE_COLS], bool absorbing) {
    goals_index = failures_index = traps_index = bounties_index = 0;

    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            if (matrix[i][j] == OBSTACLE) {
                continue;
            } else if (matrix[i][j] == goals[goals_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], goal_reward);
                fflush(NULL);
                goals_index++;
            } else if (matrix[i][j] == failures[failures_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], failure_reward);
                fflush(NULL);
                failures_index++;
            } else if(matrix[i][j] == traps[traps_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], trap_reward); 
                fflush(NULL);
                traps_index++;
            } else if (matrix[i][j] == bounties[bounties_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], bounty_reward);
                fflush(NULL);
                bounties_index++;
            } else {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"R: * : %d : * : *  %3f\n", matrix[i][j], step_reward);
                fflush(NULL);
            }
        }
    }
}

void generate_exceptions(int matrix[][TOTAL_SIZE_COLS], bool absorbing, int *special_states_array) {
    goals_index = failures_index = 0;
    int special_states_array_index = 0;
    double uniform_probability = 1.0 / (AVAILABLE_STATES_COUNT - special_states_count);
    //fprintf(stderr, "%f\nspecial_states_count:%d\navailable:%d\nresult:%d\ntest:%f\n", uniform_probability, special_states_count, AVAILABLE_STATES_COUNT, AVAILABLE_STATES_COUNT - special_states_count, uniform_probability * (AVAILABLE_STATES_COUNT - special_states_count));
    fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
    fflush(NULL);
    for (int i = PADDING_SIZE; i < MATRIX_ROWS + PADDING_SIZE; i++) {
        for (int j = PADDING_SIZE; j < MATRIX_COLS + PADDING_SIZE; j++) {
            
            if (matrix[i][j] == goals[goals_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: * : %d\n", matrix[i][j]);
                fflush(NULL);
                if (!absorbing) {
                    for (int k = 1; k <= AVAILABLE_STATES_COUNT; k++) {
                        if (k == special_states_array[special_states_array_index]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                            special_states_array_index++;
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", uniform_probability);
                            fflush(NULL);
                        }
                    }
                } else {
                    for (int k = 0; k < AVAILABLE_STATES_COUNT; k++) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    }
                }
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
                fflush(NULL);
                goals_index++;
            } else if (matrix[i][j] == failures[failures_index]) {
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: * : %d\n", matrix[i][j]);
                fflush(NULL);
                if (!absorbing) {
                    for (int k = 0; k < AVAILABLE_STATES_COUNT; k++) {
                        if (k == special_states_array[special_states_array_index]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                            special_states_array_index++;
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"%f ", uniform_probability);
                            fflush(NULL);
                        }
                    }
                } else {
                    for (int k = 1; k <= AVAILABLE_STATES_COUNT; k++) {
                        if (k == matrix[i][j]) {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"1.0 ");
                            fflush(NULL);
                        } else {
                            fprintf(absorbing ? file_absorbing : file_not_absorbing,"0.0 ");
                            fflush(NULL);
                        }
                    }
                }
                fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
                fflush(NULL);
                failures_index++;
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

    srand((unsigned int)time(NULL));

    if (args_parse(argc, argv)) {
        return 1;
    }

    TOTAL_SIZE_COLS = (MATRIX_COLS+2*PADDING_SIZE);
    TOTAL_SIZE_ROWS = (MATRIX_ROWS+2*PADDING_SIZE);
    border_obstacles_count = 2 * TOTAL_SIZE_COLS + 2 * TOTAL_SIZE_ROWS - 4;

    char *directory = (char *) malloc(50);
    input_matrix_file_string = (char*) malloc(20);

    strcpy(directory, "outputs");

    if (mkdir(directory, 0755) == -1) {
        if (errno != EEXIST) {
            perror("Error creating directory");
            return 1;
        }
    }
    
    chdir("outputs");

    int repeat_number = 1;
    char *n_abs_file_number = (char*) malloc(50);
    char *abs_file_number = (char*) malloc(50);
    
    while (repeat_number <= repeat_count) {

        sprintf(directory, "output%d/", repeat_number);
        if (mkdir(directory, 0755) == -1) {
            if (errno != EEXIST) {
                perror("Error creating directory");
                return 1;
            }
        }

        sprintf(input_matrix_file_string, "in%d.pomdp", repeat_number);
        sprintf(n_abs_file_number, "not_absorbing_%d.pomdp", repeat_number);
        sprintf(abs_file_number, "absorbing_%d.pomdp", repeat_number);

        chdir(directory);

        if (grid_generation(MATRIX_ROWS, MATRIX_COLS, input_matrix_file_string)) {
            printf("an error occured in generating\n");
            return 1;
        }
        

        FILE *input_matrix_file = fopen(input_matrix_file_string, "r+");
        

        if (input_matrix_file == NULL) {
            perror("Error opening file");
            return 1;
        }

        // pro nenáhodné vygenerovani gridu
        //long fileSize = getFileSize(file);

        int matrix[TOTAL_SIZE_ROWS][TOTAL_SIZE_COLS];

        fseek(input_matrix_file, 0, SEEK_SET);

        for (int i = 0 ; i < TOTAL_SIZE_ROWS; i++) {
            for (int j = 0 ; j < TOTAL_SIZE_COLS ; j++) {
                if((matrix[i][j] = fgetc(input_matrix_file)) == '\n'){
                    j--;
                }       
            }
        }

        
        int state_count = 0;

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

        special_states_count = goals_index + failures_index + traps_index + bounties_index;

        int sizes[] = {goals_index, failures_index, traps_index, bounties_index};


        //alokovani mista pro cisla stavu jednotlivych cilu, pasti....
        goals = malloc(goals_index * sizeof(int));
        failures = malloc(failures_index * sizeof(int));
        traps = malloc(traps_index * sizeof(int));
        bounties = malloc(bounties_index * sizeof(int));

        goals_index = failures_index = traps_index = bounties_index = 0;
        state_count = 0;

        
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

        file_not_absorbing = fopen(n_abs_file_number, "w+");

        if (file_not_absorbing == NULL) {
            perror("Error opening file");
            return 1;
        }

        bool absorbing = false;

        for (int i = 0; i < 2; i++) {
        
        
            fprintf(absorbing ? file_absorbing : file_not_absorbing, "discount: %3f\n", discount);
            fflush(NULL);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"values: reward\n");
            fflush(NULL);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"states: %d\n", AVAILABLE_STATES_COUNT);
            fflush(NULL);

            //do pole retezcu
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"actions: n s e w\n");
            fflush(NULL);

            //do pole retezcu
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"observations: left right neither both good bad\n");
            fflush(NULL);

            

            int size = sizes[0] + sizes[1] + sizes[2] + sizes[3];
            int mergedArray[size];

            mergeAndSortArrays(mergedArray, goals, failures, traps, bounties, sizes);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"start exclude: ");
            fflush(NULL);
            int marked_spot_index = 0;
            for (int i = 0; i < AVAILABLE_STATES_COUNT; i++) {
                if (i == mergedArray[marked_spot_index]) {
                    marked_spot_index++;
                    fprintf(absorbing ? file_absorbing : file_not_absorbing,"%d ", i);
                }
                
            }

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n");
            fflush(NULL);
            

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: s\n");
            fflush(NULL);
            action_south(matrix, absorbing);
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
            fflush(NULL);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: n\n");
            fflush(NULL);
            action_north(matrix, absorbing);
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
            fflush(NULL);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: e\n");
            fflush(NULL);
            action_east(matrix, absorbing);
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
            fflush(NULL);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"T: w\n");
            fflush(NULL);
            action_west(matrix, absorbing);
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
            fflush(NULL);

            generate_exceptions(matrix, absorbing, mergedArray);

            fprintf(absorbing ? file_absorbing : file_not_absorbing,"O: *\n");
            fflush(NULL);
            observations(matrix, absorbing);
            fprintf(absorbing ? file_absorbing : file_not_absorbing,"\n\n");
            fflush(NULL);

            rewards(matrix, absorbing);

            absorbing = true;
        }

        free(goals);
        free(traps);
        free(bounties);
        free(failures);


        fclose(file_absorbing);
        fclose(file_not_absorbing);
        fclose(input_matrix_file);
        chdir("..");
        repeat_number++;
        //nutno resetovat countery
        goals_index = failures_index = traps_index = bounties_index = obstacles_count = 0;
    }

    free(n_abs_file_number);
    free(abs_file_number);
    free(input_matrix_file_string);
    free(directory);

    return 0;
}

#endif
