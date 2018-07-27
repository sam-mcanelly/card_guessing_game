#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>


#define DECK_SIZE 13
#define DECISION_ROW (DECK_SIZE - 1) / 2
#define DECISION_COL (DECK_SIZE - 1) / 2
#define NUM_PLAYER_CARDS DECISION_COL
#define SMALL_ARR_SZ 4
#define FACE_CARDS 4
#define NUMERICAL_CARDS 10
#define MAX_VISIBLE_CARDS 12

enum game_type
{
    PLAYER_V_COMPUTER,
    COMPUTER_V_COMPUTER,
};

enum turn_type
{
    PLAYER_TURN,
    COMP_1_TURN,
    COMP_2_TURN,
};

enum turn_ret
{
    CONTINUE, 
    WIN,
    LOSE,
};

enum bool
{
    TRUE,
    FALSE,
};

enum ask_result
{
    HAS_CARD,
    NOT_HAS_CARD,
    CALL_BLUFF,
    GUESS_MYST,
};

typedef enum turn_type turn_type_t;
typedef enum game_type game_type_t;
typedef enum turn_ret turn_ret_t;
typedef enum bool bool_t;
typedef enum ask_result ask_result_t;

typedef struct
{
    uint8_t deck[DECK_SIZE];
    uint8_t deck_size;
    
    uint8_t myst_card;
    
    uint8_t p_deck[(DECK_SIZE - 1) / 2];
    uint8_t p_deck_size;
    
    uint8_t c_deck[(DECK_SIZE - 1) / 2];
    uint8_t c_deck_size;
    
    uint8_t visible_cards[DECK_SIZE - 1];
    uint8_t visible_card_size;
} cards;

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t ask_table_size;
    uint8_t ask1_size;
    uint8_t ask2_size;
    
    uint8_t ask1_table[(DECK_SIZE + 1) / 2];
    uint8_t ask2_table[(DECK_SIZE + 1) / 2];
    
    char comp1_turn_decisions[DECISION_ROW][DECISION_COL];
    char comp2_turn_decisions[DECISION_ROW][DECISION_COL];
    
} decisions;

//rows are opponent
//columns are current comp
char comp_call_or_guess_tbl[6][6] = {{'g', 'g', 'c', 'c', 'c', 'c' }, 
                                 {'g', 'g', 'g', 'c', 'c', 'c'}, 
                                 {'g', 'g', 'c', 'c', 'c', 'c' }, 
                                 {'g', 'g', 'g', 'g', 'c', 'c' }, 
                                 {'g', 'g', 'g', 'g', 'c', 'c' }, 
                                 {'g', 'g', 'g', 'g', 'g', 'c'}};

void get_game_type(game_type_t *type);
void whos_first(char *input);
void init_cards(cards *game_cards);
void play_game(game_type_t type);
void randomize_turn_decisions(char d[][DECISION_COL], uint8_t x, uint8_t y);
void build_ask_table(cards *deck, decisions *game_decisions, uint8_t player_num);

void shuffle_cards(uint8_t *deck, uint8_t size);
void deal_player_cards(cards *deck);
void deal_comp_cards(cards *deck, game_type_t type, uint8_t player_num);

void display_13_card_table(game_type_t type, cards *deck, bool_t show_myst);
void display_all_cards(game_type_t type, cards *deck);

void get_card_char(uint8_t card, char * output);
void get_card_int(char * input, uint8_t *output);
void remove_spaces(char * source);

turn_ret_t turn_ex(cards *deck, game_type_t g_type, turn_type_t *t_type, decisions *game_decisions);
turn_ret_t comp_turn_ex(cards *deck, turn_type_t *t_type, decisions *game_decisions, game_type_t g_type);
turn_ret_t player_turn_ex(cards *deck, turn_type_t *t_type, decisions *game_decisions);

turn_ret_t player_ask_bluff(char decision, cards *deck, turn_type_t *t_type, decisions *game_decisions);
turn_ret_t player_guess_mystery(uint8_t myst_card);
turn_ret_t comp_guess_mystery(uint8_t myst_card, uint8_t comp_guess);
ask_result_t comp_call_or_guess(cards *deck, turn_type_t *t_type);

ask_result_t ask_comp(uint8_t card_val, cards *deck, turn_type_t *t_type, decisions *game_decisions, uint8_t player_num, bool_t is_call);
ask_result_t ask_player(uint8_t card_val, cards *deck, bool_t is_call);

bool_t check_status(cards *game_cards, turn_ret_t status, turn_type_t *t_type, game_type_t g_type);

int main() 
{
    uint8_t play_again = TRUE;
    char again;
    game_type_t type;
    
    printf("+======================================+\n\n\n        GUESS THE MYSTERY CARD\n\n\n+======================================+\n\n\n");
    
    while( TRUE == play_again ) 
    {
        get_game_type(&type);
        play_game(type);
        
        printf("\n\n\nWould you like to play again?(y/n):");
        again = getchar();
        getchar();
        
        if('y' != again)
        {
            play_again = FALSE;
        }
    }
    
    return 0;
}

void init_cards(cards *game_cards)
{
    uint8_t i;
    uint8_t player_card_num = (DECK_SIZE - 1) / 2;
    
    game_cards->p_deck_size = NUM_PLAYER_CARDS;
    game_cards->c_deck_size = NUM_PLAYER_CARDS;
    game_cards->visible_card_size = 0;
    
    
    //printf("Unshuffled Cards: ");
    //populate card array
    for(i = 0; i < DECK_SIZE; i++)
    {
        game_cards->deck[i] = i + 1;
        //printf("%d, ", game_cards->deck[i]);
    }
    game_cards->deck_size = DECK_SIZE;

    printf("\nShuffling cards...\n");
    shuffle_cards(game_cards->deck, game_cards->deck_size);
    
    //debugging code
    //printf("Shuffled Cards: ");
    
    /*
    for(i = 0; i < DECK_SIZE; i++)
    {
        printf("%d, ", game_cards->deck[i]);
    }
    printf("\n");
    */
    
    //mystery card is the last card in the shuffled deck
    game_cards->myst_card = game_cards->deck[DECK_SIZE - 1];
}

void init_decisions(decisions *dec_matrices)
{
    dec_matrices->x = DECISION_ROW;
    dec_matrices->y = DECISION_COL;
    
    //randomize the computers decisions for the round
    srand(time(NULL));
    randomize_turn_decisions(dec_matrices->comp1_turn_decisions, dec_matrices->x , dec_matrices->y);
    randomize_turn_decisions(dec_matrices->comp2_turn_decisions, dec_matrices->x , dec_matrices->y);
}

void play_game(game_type_t type)
{
    uint8_t i;
    bool_t game_finished = FALSE;
    turn_ret_t status;
    turn_type_t t_type;
    char comp_first;
    
    cards game_cards;
    decisions game_decisions;
    
    
    if( PLAYER_V_COMPUTER == type )
    {
        whos_first(&comp_first);
    }
    else
    {
        comp_first = 'y';
    }
    
    init_decisions(&game_decisions);
    printf("\ndecision matrices initialized...\n");
    
    //Initialize the card structure
    init_cards(&game_cards);
    printf("\ncard struct initialized...\n");
    
    if( PLAYER_V_COMPUTER == type )
    {
        deal_player_cards(&game_cards);
        deal_comp_cards(&game_cards, type, 2);
        
        build_ask_table(&game_cards, &game_decisions, 1);
        
        switch(comp_first)
        {
            case 'y':
                //game loop
                while( FALSE == game_finished )
                {
                    t_type = COMP_1_TURN;
                    system("clear");
                    display_13_card_table(type, &game_cards, FALSE);
                    status = turn_ex(&game_cards, type, &t_type,  &game_decisions);
                    
                    game_finished = check_status(&game_cards, status, &t_type, PLAYER_V_COMPUTER);
                    
                    if(TRUE == game_finished)
                    {
                        break;
                    }
                    
                    t_type = PLAYER_TURN;
                    system("clear");
                    display_13_card_table(type, &game_cards, FALSE);
                    status = turn_ex(&game_cards, type, &t_type, &game_decisions);
                    
                    game_finished = check_status(&game_cards, status, &t_type, PLAYER_V_COMPUTER);
                    
                    if(TRUE == game_finished)
                    {
                        break;
                    }
                    
                    printf("Press enter to allow the computer to play...");
                    getchar();
                }
                break;
            case 'n':
                while( FALSE == game_finished )
                {
                    t_type = PLAYER_TURN;
                    system("clear");
                    display_13_card_table(type, &game_cards, FALSE);
                    status = turn_ex(&game_cards, type, &t_type, &game_decisions);
                    
                    game_finished = check_status(&game_cards, status, &t_type, PLAYER_V_COMPUTER);
                    
                    if(TRUE == game_finished)
                    {
                        break;
                    }
                    
                    printf("Press enter to allow the computer to play...");
                    getchar();
                    
                    t_type = COMP_1_TURN;
                    
                    system("clear");
                    display_13_card_table(type, &game_cards, FALSE);
                    status = turn_ex(&game_cards, type, &t_type, &game_decisions);
                    
                    game_finished = check_status(&game_cards, status, &t_type, PLAYER_V_COMPUTER);
                }
            break;
        }
    }
    else if( COMPUTER_V_COMPUTER == type )
    {
        printf("dealing computer player 1 cards...\n\n");
        deal_comp_cards(&game_cards, type, 1);
        printf("\n\ndealing computer player 2 cards...\n\n");
        deal_comp_cards(&game_cards, type, 2);
        
        build_ask_table(&game_cards, &game_decisions, 1);
        build_ask_table(&game_cards, &game_decisions, 2);
        
        printf("\n\ngame starting...\n\n\n\n");
        
        while( FALSE == game_finished )
        {
            t_type = COMP_1_TURN;
            system("clear");
            display_13_card_table(type, &game_cards, FALSE);
            status = turn_ex(&game_cards, type, &t_type, &game_decisions);
            
            game_finished = check_status(&game_cards, status, &t_type, COMPUTER_V_COMPUTER);
            
            if(TRUE == game_finished)
            {
                break;
            }
            
            printf("Press enter to let computer player 2 play...");
            getchar();
            
            t_type = COMP_2_TURN;
            system("clear");
            display_13_card_table(type, &game_cards, FALSE);
            printf("calling turn_ex for computer 2...\n");
            status = turn_ex(&game_cards, type, &t_type, &game_decisions);
            
            game_finished = check_status(&game_cards, status, &t_type, COMPUTER_V_COMPUTER);
            
            if(FALSE == game_finished)
            {
                printf("Press enter to let computer player 1 play...");
                getchar();
            }
        }
    }
}

bool_t check_status(cards *game_cards, turn_ret_t status, turn_type_t *t_type, game_type_t g_type)
{
    bool_t ret = FALSE;
    
    if((status == WIN && COMPUTER_V_COMPUTER == g_type && COMP_1_TURN == (*t_type)) || (status == LOSE && COMPUTER_V_COMPUTER == g_type && COMP_2_TURN == (*t_type)))
    {
        system("clear");
        display_all_cards(g_type, game_cards);
                    
        printf("\n\n\nCOMPUTER PLAYER 1 HAS WON!!!\n\n\n");
        ret = TRUE;
    }
    else if((status == WIN && COMPUTER_V_COMPUTER == g_type && COMP_2_TURN == (*t_type)) || (status == LOSE && COMPUTER_V_COMPUTER == g_type && COMP_1_TURN == (*t_type)) )
    {
        system("clear");

        display_all_cards(g_type, game_cards);      
        printf("\n\n\nCOMPUTER PLAYER 2 HAS WON!!!\n\n\n");
        ret = TRUE;
    }
    else if((status == WIN && PLAYER_V_COMPUTER == g_type && PLAYER_TURN == (*t_type)) || (status == LOSE && PLAYER_V_COMPUTER == g_type && COMP_1_TURN == (*t_type)))
    {
        system("clear");

        display_all_cards(g_type, game_cards);      
        printf("\n\n\nCONGRATULATIONS, YOU HAVE WON!!!\n\n\n");
        ret = TRUE;
    }
    else if((status == WIN && PLAYER_V_COMPUTER == g_type && COMP_1_TURN == (*t_type)) || (status == LOSE && PLAYER_V_COMPUTER == g_type && PLAYER_TURN == (*t_type)))
    {
        system("clear");

        display_all_cards(g_type, game_cards);      
        printf("\n\n\nOH NO! THE COMPUTER HAS WON!!!\n\n\n");
        ret = TRUE;
    }
    
    return ret;
}

void whos_first(char *input)
{
    printf("Would you like the computer to go first? (y/n): ");
    *input = getchar();
    getchar();
    
    
    while(1)
    {
        switch(*input)
        {
            case 'y':
                printf("The computer will go first...\n");
                return;
            case 'n':
                printf("The computer will go second...\n");
                return;
            default:
                printf("Error: invalid input. Try again\n");
                *input = getchar();
                getchar();
                break;
        }
            
    }
}

void get_game_type(game_type_t *type)
{
    char input;
    printf("Which game type would you like (c for computer vs computer or p for player vs computer)? (c/p): ");
    input = getchar();
    getchar();
    
    
    while(1)
    {
        switch(input)
        {
            case 'c':
                printf("You have chosen computer vs computer...\n");
                *type = COMPUTER_V_COMPUTER;
                return;
            case 'p':
                printf("You have chosen player vs computer...\n");
                *type = PLAYER_V_COMPUTER;
                return;
            default:
                printf("Error: invalid input. Try again\n");
                input = getchar();
                getchar();
                break;
        }
            
    }
}

void shuffle_cards(uint8_t *deck, uint8_t size)
{
    srand(time(NULL));
    if (size > 1) 
    {
        uint8_t i, j;
        for (i = 0; i < size - 1; i++) 
        {
            /*
            for(j = 0; j < size; j++)
            {
                printf("%d, ", deck[j]);
            }
            printf("i = %d\n", i);
            */
            uint8_t j = i + rand() / (RAND_MAX / (size - i) + 1);
            uint8_t t = deck[j];
            deck[j] = deck[i];
            deck[i] = t;
        }
    }
}


void randomize_turn_decisions(char d[][DECISION_COL], uint8_t x, uint8_t y)
{
    uint8_t i, j, k;
    
    char odds_1[4] = {'a', 'a', 'a', 'b'};
    char odds_2[3] = {'a', 'a', 'b'};
    
    for(i = x - (x/3); i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            k = rand() % 4;
            //printf("index (%i, j):  %c\n", k, odds_1[k]);
            d[i][j] = odds_1[k];
        }
    }
    
    for(i = 0; i < y; i++)
    {
        for(j = y - (y/3); j < y; j++)
        {
            k = rand() % 4;
            d[i][j] = odds_1[k];
        }
    }
    
    
    for(i = 0; i < x - (x/3); i++)
    {
        for(j = 0; j < y - (y/3); j++)
        {
            k = rand() % 3;
            d[i][j] = odds_2[k];
        }
    }
    
    d[0][0] = 'b';
    
    /*
    printf("Randomizing Turn Decisions...\n");
    for(i = 0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            printf("%c ", d[i][j]);
        }
        printf("\n");
    }
    
    printf("\n\n\n\n");
    */
    
}

void build_ask_table(cards *deck, decisions *game_decisions, uint8_t player_num)
{
    uint8_t i;
    
    game_decisions->ask_table_size = (DECK_SIZE + 1) / 2;
    game_decisions->ask1_size = game_decisions->ask_table_size;
    game_decisions->ask2_size = game_decisions->ask_table_size;
    
    switch(player_num)
    {
        case 1:
            for(i = 0; i < deck->p_deck_size; i++)
            {
                game_decisions->ask1_table[i] = deck->p_deck[i];
            }
            game_decisions->ask1_table[deck->p_deck_size] = deck->myst_card;
            
            //shuffle so the computer doesn't know which is which
            //(ie random guessing)
            shuffle_cards(game_decisions->ask1_table, game_decisions->ask_table_size);
            break;
        case 2:
            for(i = 0; i < deck->p_deck_size; i++)
            {
                game_decisions->ask2_table[i] = deck->c_deck[i];
            }
            game_decisions->ask2_table[deck->c_deck_size] = deck->myst_card;
            
            //shuffle so the computer doesn't know which is which
            //(ie random guessing)
            shuffle_cards(game_decisions->ask2_table, game_decisions->ask_table_size);
            break;     
    }
}

void deal_player_cards(cards *deck)
{
    uint8_t i;
    char curr_card[3];
    
    //debugging code
    /*
    printf("Your Cards: %d\n", deck->p_deck_size);
    */
    
    for(i = 0; i < NUM_PLAYER_CARDS; i++)
    {
        deck->p_deck[i] = deck->deck[i];
        
        //debugging code
        //printf("p_deck[%d] == %d\n", i, p_deck[i]);
        
        //debugging code
        //get_card_char(deck->p_deck[i], curr_card);
        //printf("%s ", curr_card);
    }
}

void deal_comp_cards(cards *deck, game_type_t type, uint8_t player_num)
{
    uint8_t i;
    uint8_t offset;
    uint8_t *deal_deck;
    uint8_t *deal_deck_size;
    char curr_card[3];
    
    switch(player_num)
    {
        case 1:
            deal_deck = deck->c_deck;
            deal_deck_size = &deck->c_deck_size;
            offset = 0;
            break;
        case 2:
            if(COMPUTER_V_COMPUTER == type)
            {
                deal_deck = deck->p_deck;
                deal_deck_size = &deck->p_deck_size;
            } 
            else
            {
                //printf("Deck is c_deck\n");
                deal_deck = deck->c_deck;
                deal_deck_size = &deck->c_deck_size;
            }
            offset = (deck->deck_size - 1) / 2;
            break;
        default:
            printf("Error! invalid player number given to deal_comp_cards! Aborting deal\n");
            return;
    }
    
    //debugging code
    //printf("Computer Cards: %d\n", *deal_deck_size);
    
    //copy the cards over into the computer's hand
    for(i = 0; i < *deal_deck_size; i++)
    {
        //debugging code
        //printf("Current index: %d\n", i);
        //printf("Card to be dealt: %d\n", deck->deck[i + offset]);
        
        deal_deck[i] = deck->deck[i + offset];
        
        //debugging code
        //get_card_char(deal_deck[i], curr_card);
        //printf("%s ", curr_card);
    }
}

turn_ret_t turn_ex(cards *deck, game_type_t g_type, turn_type_t *t_type, decisions *game_decisions)
{
    turn_ret_t status;
    
    status = CONTINUE;
    
    switch((*t_type))
    {
        case PLAYER_TURN:
            status = player_turn_ex(deck, t_type, game_decisions);
            break;
        case COMP_1_TURN:
        case COMP_2_TURN:
            //printf("calling comp_turn_ex...\n");
            status = comp_turn_ex(deck, t_type, game_decisions, g_type);
            //printf("status is %d\n", status);
            //getchar();
            break;
    }
    
    //printf("Returning %d from turn_ex...\n", game_over);
    return status;
}

turn_ret_t comp_turn_ex(cards *deck, turn_type_t *t_type, decisions *game_decisions, game_type_t g_type)
{
    uint8_t card_val;
    uint8_t *ask_table;
    uint8_t *ask_size;
    uint8_t *comp_cards;
    uint8_t comp_card_size;
    uint8_t bluff_card;
    uint8_t i, j;
    
    turn_ret_t ret = CONTINUE;
    
    ask_result_t ask_res = FALSE; 
    char decision;
    char cg_decision = ' ';
    char ask_card[3];
    
    decision = (COMP_1_TURN == (*t_type)) ? game_decisions->comp1_turn_decisions[deck->p_deck_size - 1][deck->c_deck_size - 1] : game_decisions->comp2_turn_decisions[deck->c_deck_size - 1][deck->p_deck_size - 1];
    
    comp_cards = (COMP_1_TURN == (*t_type)) ? deck->c_deck : deck->p_deck;
    comp_card_size = (COMP_1_TURN == (*t_type)) ? deck->c_deck_size : deck->p_deck_size;
    
    ask_table = (COMP_1_TURN == (*t_type)) ? game_decisions->ask1_table : game_decisions->ask2_table;
    ask_size = (COMP_1_TURN == (*t_type)) ? &game_decisions->ask1_size : &game_decisions->ask2_size; 
    
    if(0 == comp_card_size)
    {
        return CONTINUE;
    }
    
    //debug code
    //printf("Decision is %c\n", decision);
    /*
    printf("Cards held by computer %i...\n", ((*t_type) == COMP_1_TURN) ? 1 : 2);
    for(i = 0; i < comp_card_size; i++)
    {
        printf("%d ", comp_cards[i]);
    }
    printf("\n");
    */
    
    //debug code
    //printf("COMP DECISION %c\n", decision);
    
    switch(decision)
    {
        case 'a':
            //printf("computer is asking...\n");
            if(PLAYER_V_COMPUTER == g_type)
            {
                //printf("computer is asking player...\n");
                
                card_val = ask_table[*ask_size - 1];
                get_card_char(card_val, ask_card);
                printf("The computer asks if you have a %s: (press enter)", ask_card);
                getchar();
                
                //printf("asking player line 678\n");
                ask_res = ask_player(card_val, deck, FALSE);
                
                switch(ask_res)
                {
                    case HAS_CARD:
                        //ignore guessed card from array
                        (*ask_size)--;
                        return CONTINUE;
                        break;
                    case CALL_BLUFF:
                        printf("calling bluff... (press enter)\n");
                        getchar();
                        ask_res = ask_comp(card_val, deck, t_type, game_decisions, 1, TRUE);
                        if(HAS_CARD == ask_res)
                        {
                            //printf("CONTINUING LINE 694\n");
                            return CONTINUE;
                        }
                        else
                        {
                            *t_type = COMP_1_TURN;
                            return WIN;
                        }
                        break;
                    case GUESS_MYST:
                        //printf("PLAYER IS GUESSING LINE 692\n");
                        (*t_type) = PLAYER_TURN;
                        if(WIN == player_guess_mystery(deck->myst_card))
                        {
                            return WIN;
                        }
                        return LOSE;
                        break;
                }
            } 
            else if(COMPUTER_V_COMPUTER == g_type)
            {
                card_val = ask_table[(*ask_size) - 1];
                //debug code
                /*
                printf("Ask table: ");
                for(uint8_t i = 0; i < *ask_size; i++)
                {
                    printf("%i ", ask_table[i]);
                }
                */
                printf("\n");
                get_card_char(card_val, ask_card);
                //printf("LINE 747\n");
                printf("Computer %i asks if computer %i has a %s: (press enter)", (COMP_1_TURN == (*t_type)) ? 1 : 2, (COMP_1_TURN == (*t_type)) ? 2: 1,  ask_card);
                getchar();
                
                //printf("CALLING ask_comp LINE 754\n");
                ask_res = ask_comp(card_val, deck, t_type, game_decisions, ((*t_type) == COMP_1_TURN) ? 2 : 1, FALSE);
                
                switch(ask_res)
                {
                    case HAS_CARD:
                        //ignore guessed card from array
                        (*ask_size)--;
                        return CONTINUE;
                        break;
                    case CALL_BLUFF:  
                        //printf("CALLING BLUFF LINE 765\n");
                        if(NOT_HAS_CARD == ask_comp(card_val, deck, t_type, game_decisions, (COMP_1_TURN == (*t_type)) ? 2 : 1, TRUE))
                        {
                            //printf("this is an error\n");
                            *t_type = (COMP_1_TURN == (*t_type)) ? COMP_2_TURN: COMP_1_TURN;
                            return WIN;
                        }
                        return CONTINUE;
                        break;
                    case GUESS_MYST:
                        //printf("COMP IS GUESSING LINE 733\n");
                        if(WIN == comp_guess_mystery(deck->myst_card, card_val))
                        {
                            return WIN;
                        }
                        return LOSE;
                        break;
                }
            }
            break;
        case 'b':
            if(PLAYER_V_COMPUTER == g_type)
            {
                srand(time(NULL));
                bluff_card = comp_cards[0];
                //printf("bluff_card = %d", bluff_card);
                get_card_char(bluff_card, ask_card);
                
                //printf("computer is bluffing...\n");
                printf("The computer asks if you have a %s: (press enter)", ask_card);
                getchar();
                
                ask_res = ask_player(bluff_card, deck, FALSE);
                
                switch(ask_res)
                {
                    case HAS_CARD:
                        return CONTINUE;
                        break;
                    case CALL_BLUFF:
                        printf("asking comp for card %i... line 766\n", bluff_card);
                        *t_type = PLAYER_TURN;
                        if(NOT_HAS_CARD == ask_comp(bluff_card, deck, t_type, game_decisions, 1, TRUE))
                        {
                            //printf("this is an error\n");
                            *t_type = COMP_1_TURN;
                            return WIN;
                        }
                        return CONTINUE;
                        break;
                    case GUESS_MYST:
                        (*t_type) = PLAYER_TURN;
                        if(WIN == player_guess_mystery(deck->myst_card))
                        {
                            return WIN;
                        }
                        return LOSE;
                        break;
                }
            } 
            else if(COMPUTER_V_COMPUTER == g_type)
            {
                card_val = comp_cards[0];
                printf("Ask table: ");
                for(uint8_t i = 0; i < *ask_size; i++)
                {
                    printf("%i ", ask_table[i]);
                }
                printf("\n");
                get_card_char(card_val, ask_card);
                //printf("LINE 838\n");
                printf("Computer %i asks if computer %i has a %s: (press enter)", (COMP_1_TURN == (*t_type)) ? 1 : 2, (COMP_1_TURN == (*t_type)) ? 2: 1,  ask_card);
                getchar();
                
                ask_res = ask_comp(card_val, deck, t_type, game_decisions, ((*t_type) == COMP_1_TURN) ? 2 : 1, FALSE);
                
                switch(ask_res)
                {
                    case HAS_CARD:
                        return CONTINUE;
                        break;
                    case CALL_BLUFF:
                        //printf("CALLING BLUFF LINE 845...\n");
                        if(NOT_HAS_CARD == ask_comp(card_val, deck, t_type, game_decisions, (COMP_1_TURN == (*t_type)) ? 2 : 1, TRUE))
                        {
                            *t_type = (COMP_1_TURN == (*t_type)) ? COMP_2_TURN : COMP_1_TURN;
                            return WIN;
                        }
                        return CONTINUE;
                        break;
                    case GUESS_MYST:
                        //printf("COMP IS GUESSING LINE 854\n");
                        if(WIN == comp_guess_mystery(deck->myst_card, card_val))
                        {
                            return WIN;
                        }
                        return LOSE;
                        break;
                }
            }
            break;
        case 'g':
            break;
    }
    
    return ret;
}

turn_ret_t player_turn_ex(cards *deck, turn_type_t *t_type, decisions *game_decisions)
{
    turn_ret_t ret = CONTINUE;
    char decision = '0';
    
    while(decision != 'a' && decision != 'b' && decision != 'g')
    {
        printf("Would you like to Ask, Bluff, or Guess the Mystery Card?(a/b/g): ");
        decision = getchar();
        getchar();
        
        if(decision != 'a' && decision != 'b' && decision != 'g')
        {
            printf("Error! Invalid input! Try again...\n");
        }
    }
    
    switch(decision)
    {
        case 'a':
        case 'b':
            ret = player_ask_bluff(decision, deck, t_type, game_decisions);
            break;
        case 'g':
            //printf("calling guess function...\n");
            ret = player_guess_mystery(deck->myst_card);
            break;
    }
    
    //printf("Returning %d from player_turn_ex...\n", ret);
    return ret;
}

turn_ret_t player_ask_bluff(char decision, cards *deck, turn_type_t *t_type, decisions *game_decisions)
{
    uint8_t i;
    uint8_t card_val;
    
    char input[10];
    char compare_string[3];
    
    ask_result_t res;
    
    *t_type = PLAYER_TURN;

    printf("\nEnter the card value you want to %s(2..10 [J, Q, K, A]): ", (decision == 'a') ? "ask" : "bluff");
    fgets(input, sizeof(input), stdin);
        
    //copy input into the compare string
    compare_string[0] = input[0];
    compare_string[1] = input[1];
    compare_string[3] = '\0';
    
    //debugging code
    //printf("Input: %s\n", compare_string);
    
    get_card_int(compare_string, &card_val);     
    
    //printf("Card Int Val: %d\n", card_val);
    
    //printf("CALLING ask_comp LINE 897\n");
    res = ask_comp(card_val, deck, t_type, game_decisions, 1, FALSE);
    
    switch(res)
    {
        case HAS_CARD:
        case NOT_HAS_CARD:
            //printf("ERROR LINE 904\n");
            return CONTINUE;
            break;
        case CALL_BLUFF:
            printf("comp 1 is calling your bluff...\n");
            if(NOT_HAS_CARD == ask_player(card_val, deck, TRUE))
            {
                *t_type = PLAYER_TURN;
                return WIN;
            }
            break;
        case GUESS_MYST:
            //printf("COMP IS GUESSING LINE 903\n");
            (*t_type) = COMP_1_TURN;
            if(WIN == comp_guess_mystery(deck->myst_card, card_val))
            {
                return WIN;
            }
            return LOSE;
            break;
    }
}

turn_ret_t player_guess_mystery(uint8_t myst_card)
{
    uint8_t card_val;
    
    char input[10];
    char compare_string[3];
    
    printf("\nEnter the card value you want to guess(2..10 [J, Q, K, A]): ");
    
    fgets(input, sizeof(input), stdin);
        
    //copy input into the compare string
    compare_string[0] = input[0];
    compare_string[1] = input[1];
    compare_string[3] = '\0';
    
    get_card_int(compare_string, &card_val);
    
    if(card_val == myst_card)
    {
        //printf("Player guessed correctly...\n");
        return WIN;
    }
    
    
    //printf("Player guessed incorrectly...\n");
    return LOSE;
}

turn_ret_t comp_guess_mystery(uint8_t myst_card, uint8_t comp_guess)
{
    if(comp_guess == myst_card)
    {
        return WIN;
    }
    
    return LOSE;
}

ask_result_t comp_call_or_guess(cards *deck, turn_type_t *t_type)
{
    char decision;
    
    decision = ((*t_type) == COMP_1_TURN) ? comp_call_or_guess_tbl[deck->p_deck_size][deck->c_deck_size] : comp_call_or_guess_tbl[deck->c_deck_size][deck->p_deck_size];
    
    switch(decision)
    {
        case 'c':
            //printf("Comp has decided to call...\n");
            return CALL_BLUFF;
            break;
        case 'g':
            printf("Comp has decided to guess the mystery card...\n");
            return GUESS_MYST;
            break;
        default:
            return CALL_BLUFF;
    }
}

ask_result_t ask_comp(uint8_t card_val, cards *deck, turn_type_t *t_type, decisions *game_decisions, uint8_t player_num, bool_t is_call)
{
    uint8_t i;
    uint8_t temp;
    uint8_t *comp_deck;
    uint8_t *comp_deck_size;
    
    ask_result_t ret = CALL_BLUFF;
    
    turn_type_t t_temp;
    
    switch(player_num)
    {
        case 1:
            comp_deck = deck->c_deck;
            comp_deck_size = &deck->c_deck_size;
            break;
        case 2:
            comp_deck = deck->p_deck;
            comp_deck_size = &deck->p_deck_size;
            break;
    }
    
    for(i = 0; i < *comp_deck_size; i++)
    {
        if( card_val == comp_deck[i] )
        {
            //if the card is found, swap it with the last card in the array
            //then decrement comp_deck_size
            temp = comp_deck[*comp_deck_size - 1];
            comp_deck[*comp_deck_size - 1] = comp_deck[i];
            comp_deck[i] = temp;
            
            deck->visible_cards[deck->visible_card_size++] = comp_deck[*comp_deck_size - 1];
            
            //printf("c_deck_size: %d\n", *comp_deck_size);
            (*comp_deck_size)--;
            //printf("c_deck_size: %d\n", *comp_deck_size);
            
            printf("Computer %i is giving up the card...\n", player_num);
            
            return HAS_CARD;
        }
    }
    
    if(FALSE == is_call)
    {
        //p_deck can be substituted for p_deck here
        printf("comp %i is calling or guessing because it didn't have the card it was asked for...\n", player_num);
        ret = comp_call_or_guess(deck, t_type);
        
        switch(*t_type)
        {
            case COMP_1_TURN:
                *t_type = COMP_2_TURN;
                break;
            case COMP_2_TURN:
                *t_type = COMP_1_TURN;
                break;
        }
        
        switch(ret)
        {
            case CALL_BLUFF:
                printf("comp %i is calling...\n", player_num);
                break;
            case GUESS_MYST:
                printf("comp %i is guessing... (press enter)\n", player_num);
                getchar();
                break;
        }
        
        return ret;
    }
    else if(TRUE == is_call)
    {
        printf("comp %i did not bluff... (press enter)\n", player_num);
        getchar();
        ret = NOT_HAS_CARD;
        
        //tell both computers to guess on the next turn because the mystery card is known
        comp_call_or_guess_tbl[deck->p_deck_size][deck->c_deck_size] = 'g';
        comp_call_or_guess_tbl[deck->c_deck_size][deck->p_deck_size] = 'g';
        
        game_decisions->ask1_table[(game_decisions->ask1_size) - 1] = card_val;
        game_decisions->ask2_table[(game_decisions->ask2_size) - 1] = card_val;
    }
    
    return ret;
}

ask_result_t ask_player(uint8_t card_val, cards *deck, bool_t is_call)
{
    uint8_t i;
    uint8_t temp;
    char cg_decision;
    
    for(i = 0; i < deck->p_deck_size; i++)
    {
        if( card_val == deck->p_deck[i] )
        {
            //if the card is found, swap it with the last card in the array
            //then decrement comp_deck_size
            temp = deck->p_deck[deck->p_deck_size - 1];
            deck->p_deck[deck->p_deck_size - 1] = deck->p_deck[i];
            deck->p_deck[i] = temp;
            
            deck->visible_cards[deck->visible_card_size++] = deck->p_deck[deck->p_deck_size - 1];
            
            deck->p_deck_size--;
            
            return HAS_CARD;
        }
    }
    
    if(FALSE == is_call)
    {
        printf("It appears you don't have that card. Would you like to call the bluff or guess the mystery card? (c/g): " );
        cg_decision = getchar();
        getchar();
    
        switch(cg_decision)
        {
            case 'c':
                return CALL_BLUFF;
                break;
            case 'g':
                return GUESS_MYST;
                break;
            default:
                return CALL_BLUFF;
        }
    }
    else if(TRUE == is_call)
    {
        printf("You do not have that card...\n");
        return NOT_HAS_CARD;
    }
}


void display_13_card_table(game_type_t type, cards *deck, bool_t show_myst)
{
    uint8_t j;
    uint8_t v_count = 12;
    uint8_t total_visible_cards = 0;
    uint8_t tot_cards = 13;
    uint8_t player_card_count = (tot_cards - 1) / 2;
    
    char temp[3];
    char all_cards[tot_cards + v_count][3];
    
    //if the player still has the card in their posession, don't show it
    //if game type is computer vs computer, show it
    for(j = 0; j < deck->c_deck_size; j++)
    {
        strcpy(all_cards[j], "  ");
        
        if(COMPUTER_V_COMPUTER == type)
        {
            get_card_char(deck->c_deck[j], temp);
            strcpy(all_cards[j], temp);
        }
    }
    
    //else, show it is no longer used
    for(j = deck->c_deck_size; j < player_card_count; j++)
    {   
        strcpy(all_cards[j], " X");
    }
    
    //show or don't show the mystery card based on the game type
    if( PLAYER_V_COMPUTER == type)
    {
        if(TRUE == show_myst)
        {
            get_card_char(deck->myst_card, temp);
            strcpy(all_cards[player_card_count], temp);
        }
        else
        {
            strcpy(all_cards[player_card_count], "  ");
        }
    }
    else
    {
        get_card_char(deck->myst_card, temp);
        strcpy(all_cards[player_card_count], temp);
    }
        
    //repeat the above procedure for the second player
    for(j = 0; j < deck->p_deck_size; j++)
    {
        get_card_char(deck->p_deck[j], temp);
        strcpy(all_cards[j + 19], temp);
        
        //debugging code
        //printf("current card: %s\ncard pos: %d\n\n", temp, j + 19);
    }
    
    //else, show it is no longer used
    for(j = deck->p_deck_size; j < player_card_count; j++)
    {
        strcpy(all_cards[j + 19], " X");
    }
    
    //fill in visible cards
    for(j = 0; j < deck->visible_card_size; j++)
    {
        get_card_char(deck->visible_cards[j], temp);
        strcpy(all_cards[j + 7], temp);
    }
    
    //Fill the rest of the "cards visible to all" that don't show anything
    for(j = 7 + deck->visible_card_size; j < 19; j++ )
    {
        strcpy(all_cards[j], "  ");
    }
    
    
    printf
        ("+-------------------------------------------------+\n|     ___   ___   ___   ___   ___   ___           |\n|    |   | |   | |   | |   | |   | |   |          |\n|    |%s | |%s | |%s | |%s | |%s | |%s |          |\n|    |   | |   | |   | |   | |   | |   |          |\n|     ---   ---   ---   ---   ---   ---           |\n|                                                 |\n|                                        mystery  |\n|                                          ___    |\n|                                         |   |   |\n|                                         |%s |   |\n|                                         |   |   |\n|                                          ---    |\n|            cards visible to all                 |\n|     ___   ___   ___   ___   ___   ___           |\n|    |   | |   | |   | |   | |   | |   |          |\n|    |%s | |%s | |%s | |%s | |%s | |%s |          |\n|    |   | |   | |   | |   | |   | |   |          |\n|     ---   ---   ---   ---   ---   ---           |\n|     ___   ___   ___   ___   ___   ___           |\n|    |   | |   | |   | |   | |   | |   |          |\n|    |%s | |%s | |%s | |%s | |%s | |%s |          |\n|    |   | |   | |   | |   | |   | |   |          |\n|     ---   ---   ---   ---   ---   ---           |\n|                                                 |\n|                                                 |\n|                                                 |\n|                                                 |\n|                                                 |\n|     ___   ___   ___   ___   ___   ___           |\n|    |   | |   | |   | |   | |   | |   |          |\n|    |%s | |%s | |%s | |%s | |%s | |%s |          |\n|    |   | |   | |   | |   | |   | |   |          |\n|     ---   ---   ---   ---   ---   ---           |\n|                                                 |\n+-------------------------------------------------+\n" 
         , all_cards[0], all_cards[1], all_cards[2], all_cards[3], all_cards[4], all_cards[5], all_cards[6], all_cards[7], all_cards[8], all_cards[9], all_cards[10], all_cards[11], all_cards[12], all_cards[13], all_cards[14], all_cards[15], all_cards[16], all_cards[17], all_cards[18], all_cards[19], all_cards[20], all_cards[21], all_cards[22], all_cards[23], all_cards[24]);
    
    //getchar();
}

void display_all_cards(game_type_t type, cards *deck)
{
    uint8_t i;
    
    for(i = 0; i < deck->p_deck_size; i++)
    {
        deck->visible_cards[deck->visible_card_size++] = deck->p_deck[i];
    }
    for(i = 0; i < deck->c_deck_size; i++)
    {
        deck->visible_cards[deck->visible_card_size++] = deck->c_deck[i];
    }
    
    deck->p_deck_size = 0;
    deck->c_deck_size = 0;
     
    display_13_card_table(type, deck, TRUE);
}

void get_card_char(uint8_t card, char * output)
{   
    if(card < 11 && card > 1)
    {
        if(10 == card)
        {
            strcpy(output, "10");
        }
        else
        {
            output[0] = ' ';
            output[1] = (char)(card + '0');
            output[2] = '\0';
        }
    }
    else
    {
        switch(card)
        {
            case 1:
                strcpy(output, " A");
                break;
            case 11:
                strcpy(output, " J");
                break;
            case 12:
                strcpy(output, " Q");
                break;
            case 13:
                strcpy(output, " K");
                break;
            default:
                strcpy(output, "  ");
        }
    }
    
    //debugging code
    //printf("%s", output);
    
}

void get_card_int(char * input, uint8_t *output)
{
    uint8_t i;
    
    char face_cards[] = {'A', 'J', 'Q', 'K'};
    uint8_t face_val[] = {1, 11, 12, 13};
    
    //debug code
    //printf("Card input: %s\n", input);
    *output = (uint8_t)atoi(input);
    
    //debug code
    //printf("Converted card: %d\n", *output);
    
    if((*output) == 0)
    {
        remove_spaces(input);
        
        //printf("input with spaces removed: %s\n", input);
        
        for(i = 0; i < FACE_CARDS; i++)
        {
            if(input[0] == face_cards[i])
            {
                *output = face_val[i];
                return;
            }
        }
        
        printf("Error! Invalid input...\n");
    }
}

void remove_spaces(char * source)
{
  char* i = source;
  char* j = source;
    
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}
