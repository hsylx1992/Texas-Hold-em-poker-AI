#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//#include "cJSON.h"
//#include "player.h"
#include "../communication/msg_process.h"

#define NUM_HAND        2
#define NUM_COMMUNITY   5
#define NUM_DECK        53 //cards are 1-indexed
#define MAX_OPPONENTS   10

/*
#ifdef WORD_SIZE
#define MAX_NAME_LEN    WORD_SIZE
#else
#define MAX_NAME_LEN    50
#endif
*/
#define MAX_NAME_LEN    20

typedef struct player
{
    char name[MAX_NAME_LEN + 1];
    int initial_stack;
    int current_bet;
    int stack;
    bool folded;
} Player;

typedef enum PosType_e
{	
	POSTYPE_FRONT = 1,
	POSTYPE_MIDDLE = 2,
	POSTYPE_BACK = 3,
	POSTYPE_SB = 5,
	POSTYPE_BB = 6,
	
	POSTYPE_ERROR
} PosType;

typedef enum phase_e
{
    PHASE_DEAL,
    PHASE_FLOP,
    PHASE_TURN,
    PHASE_RIVER,
    
    PHASE_ERROR
} Phase;

typedef struct playerinfo__t
{
	int agressive;	// 1 fold；2 保守进攻；3 一般；4 激进进攻；5 all_in；6 random
	int name_id;
	char name[MAX_NAME_LEN + 1];
} PlayerInfo;

typedef struct gamestate
{
	char name[MAX_NAME_LEN + 1];
    int round_id;
    int initial_stack;
    int stack;
    int current_bet;
    int call_amount;
    int raise_amount;
    int current_pot;
    Phase phase;
    int hand[NUM_HAND];
    int handsize;
    bool your_turn;
    Player opponents[MAX_OPPONENTS];
    int num_opponents;
    int num_playing;	//从inquire中获取的没有fold的玩家数量
    int num_call_beforeme;
    int num_fold_beforeme;
    int num_raise_beforeme;
    int num_allin_beforeme;
    int community[NUM_COMMUNITY];
    int communitysize;
    bool deck[NUM_DECK];
    
    int pos;	//位置
    int money;  //剩余的金币
    int BB;		//大盲注
    int my3BB;	//raise最大值为my3BB = fun(stack, BB) < 3*BB
    int crt_player_no;	//从seat中获取的玩家数量
    int first_inquire;
    int first_call;	//第一次叫注动作。SB BB第一次是好位置，后来是坏位置
    				//第一次叫注后 =0
    int call_no;		//call or raise的次数
    int raise_no;
    int raise_no_c;	// 连续加注次数
    //int call_amount;    //累计叫注值
    int isSpecialPos;	//0 一般； 5 SB； 6 BB； 7 button
    PosType pos_type;
    Phase phase_last;

	int my_pos;
	int beforeme_least_stack;
	int behindme_best_stack;	
	int sum_other_stack;
	int same_stack;
	
	int blind_no;
    
} GameState;

/*
 * Set the game state according to the given JSON
 * game: the game state to set
 * json: a JSON representation of the game state
 */
void SetGameState(GameState *game, stMsg_t stOneMsg);

/*
 * Update the game's deck based on
 * the game's hand and community cards
 * game: the game to update
 */
void UpdateGameDeck(GameState *game);

/*
 * Create an int representing the given card
 * card: the string representation of the card
 * return: the representation of this card as an int
 */
int StringToCard(char *card);


/*
 * Print the current table information
 * game: the game state containing the table information
 * logfile: the file for logging output
 */
void PrintTableInfo(GameState *game, FILE *logfile);

/*
 * Print the AI's cards to the given file
 * game: the game state containing the cards
 * logfile: the file for logging output
 */
void PrintCards(GameState *game, FILE *logfile);

/*
 * Print the current players at the table and their information
 * game: the game state containing the players
 * logfile: the file for logging output
 */
void PrintPlayers(GameState *game, FILE *logfile);

#endif
