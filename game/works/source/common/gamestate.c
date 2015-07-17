#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamestate.h"
//#include "../communication/com_global.h"

//Maps an integer to a card string
char *CARDS[] = {"XX",                                                  \
"2S", "2C", "2D", "2H", "3S", "3C", "3D", "3H", "4S", "4C", "4D", "4H", \
"5S", "5C", "5D", "5H", "6S", "6C", "6D", "6H", "7S", "7C", "7D", "7H", \
"8S", "8C", "8D", "8H", "9S", "9C", "9D", "9H", "TS", "TC", "TD", "TH", \
"JS", "JC", "JD", "JH", "QS", "QC", "QD", "QH", "KS", "KC", "KD", "KH", \
"AS", "AC", "AD", "AH"                                                  \
};

/*
 * Convert a string phase to the enum phase type
 * phase: the string representation of the current phase
 * return: a Phase representing the current game phase
 */
static
Phase GetPhase(char *phase)
{
    if (!strcasecmp(phase, "deal"))
    {
        return PHASE_DEAL;
    }
    else if (!strcasecmp(phase, "flop"))
    {
        return PHASE_FLOP;
    }
    else if (!strcasecmp(phase, "turn"))
    {
        return PHASE_TURN;
    }
    else if (!strcasecmp(phase, "river"))
    {
        return PHASE_RIVER;
    }
    else
    {
        return PHASE_ERROR;
    }
}

/*
 * Remove cards from the deck that have already been seen in play
 * deck: the deck to remove cards from
 * cards: the cards to remove
 * numcards: the number of cards to remove
 */
static inline
void RemoveCardsFromDeck(bool *deck, int *cards, int numcards)
{
    for (int i = 0; i < numcards; i++)
    {
        deck[cards[i]] = false;
    }
}


void
HoldCardMsgRowToChar3(char *card, char color[10], char point[5])
{	
    
    if (0 == strcasecmp(point, "10")) {
    	card[0] = 'T';
    } else {
    	card[0] = point[0];
    }
	
	if (0 == strcasecmp(color, "DIAMONDS")) {
		card[1] = 'D';
	} else if (0 == strcasecmp(color, "CLUBS")) {
		card[1] = 'C';
	} else if (0 == strcasecmp(color, "HEARTS")) {
		card[1] = 'H';
	} else {	// "SPADES"
		card[1] = 'S';
	}
	card[2] = '\0';
	
}

// set game->hand[0] game->hand[1]
void
SetGameStateHold(GameState *game, stMsg_t stOneMsg)
{
	if (stOneMsg.m_nRowNo < 2) {
		printf("ERROR wrong row number in hold msg! \n");
	}
	
	game->handsize = 2;
	game->your_turn = 0;
	game->phase_last = game->phase;
	game->phase = GetPhase("deal");
	
	game->call_no = 0;
	game->raise_no = 0;
	
	for (int i = 0; i < 2; ++i) {
		stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[i]);
		char card[3] = {'\0'};		
		HoldCardMsgRowToChar3(card, stMsgRow.m_szWordRest[0], stMsgRow.m_szWordRest[1]);
		game->hand[i] = StringToCard(card);
	}
}

void
SetGameStateFlop(GameState *game, stMsg_t stOneMsg)
{
	if (stOneMsg.m_nRowNo < 3) {
		printf("ERROR wrong row number in flop msg! \n");
	}	
	
	game->communitysize = 3;
	game->your_turn = 0;
	game->phase_last = game->phase;
	game->phase = GetPhase("flop");

	game->call_no = 0;
	game->raise_no = 0;
		
	for (int i = 0; i < 3; ++i) {
		stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[i]);
		char card[3] = {'\0'};		
		HoldCardMsgRowToChar3(card, stMsgRow.m_szWordRest[0], stMsgRow.m_szWordRest[1]);
		game->community[i] = StringToCard(card);
	}
}

void
SetGameStateTurn(GameState *game, stMsg_t stOneMsg)
{
	if (stOneMsg.m_nRowNo < 1) {
		printf("ERROR wrong row number in flop msg! \n");
	}	

	game->communitysize = 4;
	game->your_turn = 0;
	game->phase_last = game->phase;
	game->phase = GetPhase("turn");

	game->call_no = 0;
	game->raise_no = 0;
			
	stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[0]);
	char card[3] = {'\0'};		
	HoldCardMsgRowToChar3(card, stMsgRow.m_szWordRest[0], stMsgRow.m_szWordRest[1]);
	game->community[4-1] = StringToCard(card);
}

void
SetGameStateRiver(GameState *game, stMsg_t stOneMsg)
{
	if (stOneMsg.m_nRowNo < 1) {
		printf("ERROR wrong row number in flop msg! \n");
	}
	
	game->communitysize = 5;
	game->your_turn = 0;
	game->phase_last = game->phase;
	game->phase = GetPhase("river");

	game->call_no = 0;
	game->raise_no = 0;
			
	stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[0]);
	char card[3] = {'\0'};		
	HoldCardMsgRowToChar3(card, stMsgRow.m_szWordRest[0], stMsgRow.m_szWordRest[1]);
	game->community[5-1] = StringToCard(card);
}


//SetGameOpponents
//current pot
void
SetGameStateInquire(GameState *game, stMsg_t stOneMsg)
{
	int n = stOneMsg.m_nRowNo;

//	printf("----%d-----\n", n);
	game->your_turn = 1;
	game->num_opponents = n-1;			//最后一行是total pot
	
	int playing 	= 0;
  	int numcall		= 0;
    int numfold		= 0;
    int numraise	= 0;
    int numallin    = 0;
    
	int raise_amount= 0;
	int past_bet_2 	= 0;
	int past_bet_1 	= 0;		
	int raise_record= 0;		    	
	
	//while (i<n) {
	for (int i = 0; i < n-1; ++i) {			
		stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[i]);		
		
		char *name = stMsgRow.m_szWordRest[0];
		int  initial_stack = 2000;
		int  stack = atoi(stMsgRow.m_szWordRest[1]);
		int  money = atoi(stMsgRow.m_szWordRest[2]);
		int  current_bet = atoi(stMsgRow.m_szWordRest[3]);
		char *action = stMsgRow.m_szWordRest[4];
		
		// 如果包含的自己的信息，自己总是在最后一个
		if (0 == strcasecmp(name, game->name)) {
			game->money = money;
			game->num_opponents--;
			//name
			game->stack = stack;
			game->current_bet = current_bet;	//上一次的
						
			break;
		}
		
		//+++++++++++++++++++++++++++++++++++++++++++++
		if (!raise_record) {
			if ((0 == strcasecmp(action, "check")) 	|| 
				(0 == strcasecmp(action, "call") )	||
				(0 == strcasecmp(action, "blind")) 	){
				if (!past_bet_1) {
					past_bet_1 = 0;
					past_bet_2 = 0;
				} else {
					// if (past_bet_1)
					past_bet_2   = current_bet;					
				}
			
			} else if (	(0 == strcasecmp(action, "raise") )	||
						(0 == strcasecmp(action, "all_in"))	){
				if (!past_bet_1) {
					past_bet_1 = current_bet;
				} else {
					// if (past_bet_1)
					past_bet_2 = current_bet;
				}

			} else {
				// fold 不处理
			}
			
			if (past_bet_1 && past_bet_2) {
				raise_amount = past_bet_1 - past_bet_2;
				raise_record = 1;
			}
		}
		//-----------------------------------------------
		if (0 == strcasecmp(action, "check")) {
			//
		} else if (0 == strcasecmp(action, "call")) {
			numcall++;
		} else if (0 == strcasecmp(action, "raise")) {
			numraise++;
		} else if (0 == strcasecmp(action, "all_in")) {
			//all_in = 1;
			numraise++;
			numallin++;
		} else if (0 == strcasecmp(action, "blind")) {			
		} else {
			
		}

		int fold = 0;				
		if (0 == strcasecmp(action, "fold")) {
			fold = 1;
			numfold++;
		} else {
			playing++;
		}						
        strncpy(game->opponents[i].name, name, MAX_NAME_LEN);
        game->opponents[i].initial_stack = initial_stack;
        game->opponents[i].folded = fold;      
        game->opponents[i].stack  = stack;
        game->opponents[i].current_bet = current_bet;
               
	} // while()
	
	// 加注值
	game->raise_amount = raise_amount;
	
	// 跟注值
	int call_amount = 0;
	for (int i = 0; i < game->num_opponents; i++) {
		int inter = game->opponents[i].current_bet - game->current_bet;
		if (inter > call_amount) {
			call_amount = inter;
		}
	}
	game->call_amount = call_amount;
	
	//simu player
	if (game->first_inquire) {
		playing = game->crt_player_no;
		game->first_inquire = 0;
	}	
	game->num_playing        = playing;
    game->num_call_beforeme  = numcall;
    game->num_fold_beforeme  = numfold;
    game->num_raise_beforeme = numraise;
    game->num_allin_beforeme = numallin;
    	
	//total pot
	stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[n-1]);
	if (0 != strcasecmp(stMsgRow.m_szWordBegin, "total pot")) {
		printf("ERROR wrong in inquire msg! \n");
	} else {
		game->current_pot = atoi(stMsgRow.m_szWordRest[0]);
	}
}

void
SetGameStateShowdown(GameState *game, stMsg_t stOneMsg)
{
	;
}

/*
void
SetGameStateSeat(GameState *game, stMsg_t stOneMsg)
{
	;
}
*/

void
SetGameStateBlind(GameState *game, stMsg_t stOneMsg)
{
	//puts("state blind");
#if 0	
	stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[0]);		
	int SB = atoi(stMsgRow.m_szWordRest[0]);
	game->BB = 2*SB;
	
	//在AI部分处理my3BB 默认120
	//保证按3BB的方式能玩5次 钱多时激进些
	//执行3BB策略的概率不高	
	if (10*game->BB <= game->stack && game->stack <= 100*game->BB) {
		game->my3BB = game->BB*3;
	} else {
		game->my3BB = game->stack/5;
	}
#endif	
	//初赛不涨盲注
	
}

int
TranPos(int pos, int blind_no, int player_no) {
	int ret;
	
	if (player_no == 1) {
		ret = 1;
	} else if (player_no == 3 || player_no == 2) {
		ret = pos;
	} else {
		if (pos > blind_no+1) {
			ret = pos - 3;
		} else if (pos <= blind_no+1){
			ret = pos + player_no - blind_no - 1;
		}
	}
	//printf("[%d]%d %d %d\n", ret, pos, blind_no, player_no);
	return ret;
}

//除了blind之外的人数
//末尾补两个1防止溢出，第一个1不使用，因为1<=pos<=8
PosType except_bind_pos_type_6[] = {1, 1, 1, 2, 2, 3, 3, 1, 1};
PosType except_bind_pos_type_5[] = {1, 1, 1, 2, 2, 3, 1, 1};
PosType except_bind_pos_type_4[] = {1, 1, 1, 2, 3, 1, 1, 1};
PosType except_bind_pos_type_3[] = {1, 1, 2, 3};
PosType except_bind_pos_type_2[] = {1, 1, 2};
PosType except_bind_pos_type_1[] = {1, 1};

PosType
EvalPos(int pos, int is_blind, int num_blinding, int num_playing)
{	

	if (is_blind == 5) {
		return POSTYPE_SB;
	}
	if (is_blind == 6) {
		return POSTYPE_BB;
	}
	
	//不是盲注pos，因此
	if (num_playing-num_blinding>6) {		
		puts("EvalPos Error");
		return POSTYPE_MIDDLE;		
	}	
	
	int n = num_playing - num_blinding;
	
	if (n == 6) {
		return except_bind_pos_type_6[pos];
	} else if (n == 5) {
		return except_bind_pos_type_5[pos];
	} else if (n == 4) {
		return except_bind_pos_type_4[pos];
	} else if (n == 3) {
		return except_bind_pos_type_3[pos];
	} else if (n == 2) {
		return except_bind_pos_type_2[pos];
	} else if (n == 1) {
		return except_bind_pos_type_1[pos];
	} else {
		return POSTYPE_MIDDLE;
	}
}


void
SetGameStateRoundStart(GameState *game, stMsg_t stOneMsg)
{
	//puts("SetGameStateRoundStart");
	game->round_id    += 1;
	game->current_bet  = 0;
	game->call_amount  = 0;
	game->current_pot  = 0;
	game->phase        = GetPhase("seat");

	bzero(game->hand, sizeof(game->hand));
	game->handsize     = 0;	
	
	game->your_turn    = 0;
	
	bzero(game->opponents, sizeof(game->opponents));
	game->num_opponents= 0;	
	game->crt_player_no  = stOneMsg.m_nRowNo;
	
	bzero(game->community, sizeof(game->community));
	game->communitysize= 0;

	game->crt_player_no = stOneMsg.m_nRowNo;//！！！玩家数也就是位置消息中带有的行数
	game->first_inquire = 1;
	game->first_call    = 1;	
	game->call_no       = 0;
	game->raise_no      = 0;
	game->isSpecialPos  = 0;
	game->pos_type = POSTYPE_FRONT;
	
	game->phase_last    = GetPhase("seat");
	game->phase	        = GetPhase("seat");
	
	memset(game->deck, 1, sizeof(game->deck) / sizeof(game->deck[0]));
	
	//===================================================================
	int blind_no = 0;
	int pos = 0;
	int isReachMyinfo = 0;
	
	int n = stOneMsg.m_nRowNo;
	int money_stack[8] = {0};
	int money_stack_pos = 0;
	
	//while (i<stOneMsg.m_nRowNo) {
	for (int i = 0; i < stOneMsg.m_nRowNo; ++i) {
		stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[i]);		

		if (0 == strcasecmp(stMsgRow.m_szWordBegin, "small blind") ||
			0 == strcasecmp(stMsgRow.m_szWordBegin, "big blind") ) {
			blind_no ++;	
		}
		if (!isReachMyinfo) {
			pos++;
		}						
								
		char *name = stMsgRow.m_szWordRest[0];
		if (0 == strcasecmp(name, game->name)) {				
		
			int  stack = atoi(stMsgRow.m_szWordRest[1]);
			int  money = atoi(stMsgRow.m_szWordRest[2]);	
			game->stack = stack;
			game->money = money;
			
			isReachMyinfo = 1;	
				
			if (0 == strcasecmp(stMsgRow.m_szWordBegin, "small blind")) {
				game->isSpecialPos = 5;
			} else if (0 == strcasecmp(stMsgRow.m_szWordBegin, "big blind")) {
				game->isSpecialPos = 6;
			} else if (pos == 1){
				game->isSpecialPos = 7;
			} else {
				game->isSpecialPos = 0;
			}
			
			continue;
		}

		money_stack[money_stack_pos++] = atoi(stMsgRow.m_szWordRest[1])+atoi(stMsgRow.m_szWordRest[2]);
	}

	game->blind_no = blind_no;
	game->pos      = TranPos(pos, blind_no, stOneMsg.m_nRowNo);	
	game->pos_type = EvalPos(game->pos, game->isSpecialPos, blind_no, stOneMsg.m_nRowNo);
	//===================================================================
	int my_money_stack		 = game->stack + game->money;
	int my_pos 				 = 1;
	int beforeme_least_stack = my_money_stack;
	int behindme_best_stack  = 0;
	int sum_other_stack 	 = 0;	
	int same_stack 			 = 0;
		
	for (int i = 0; i < n-1; i++) {
		//printf("%d\n", i);
		if (my_money_stack == money_stack[i]) {
			same_stack++;
		} else if (my_money_stack < money_stack[i]) {
			if (beforeme_least_stack > money_stack[i]) {
				beforeme_least_stack = money_stack[i];
			}
			my_pos++;
		} else {
			if (behindme_best_stack  < money_stack[i]) {
				behindme_best_stack  = money_stack[i];
			}				
		}
		sum_other_stack += money_stack[i];
	}
	
	game->my_pos 				= my_pos;
	game->beforeme_least_stack 	= beforeme_least_stack;
	game->behindme_best_stack  	= behindme_best_stack;	
	game->sum_other_stack	  	= sum_other_stack;
	game->same_stack			= same_stack;		
}

void
SetGameStateSeatInfo(GameState *game, stMsg_t stOneMsg)
{
	;
}

/*
 * Create an int representing the given card
 * card: the string representation of the card
 * return: the representation of this card as an int
 */
int StringToCard(char *card)
{
    int val;
    char c = card[0];
    if (c >= '2' && c <= '9')
    {
        val = 4 * (c - '2') + 1;
    }
    else if (c == 'T')
    {
        val = 33;
    }
    else if (c == 'J')
    {
        val = 37;
    }
    else if (c == 'Q')
    {
        val = 41;
    }
    else if (c == 'K')
    {
        val = 45;
    }
    else // c == 'A'
    {
        val = 49;
    }

    c = card[1];
    if (c == 'C')
    {
        val += 1;
    }
    else if (c == 'D')
    {
        val += 2;
    }
    else if (c == 'H')
    {
        val += 3;
    }
    //if c == 'S', val += 0

    return val;
}


/*
 * Set the game state according to the given JSON
 * game: the game state to set
 * json: a JSON representation of the game state
 */
void SetGameState(GameState *game, stMsg_t stOneMsg)
{
	if (0 == strcasecmp(stOneMsg.m_szType, "seat")){
		SetGameStateRoundStart(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "hold")) {
		SetGameStateHold(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "flop")) {
		SetGameStateFlop(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "turn")) {
		SetGameStateTurn(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "river")) {
		SetGameStateRiver(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "inquire")) {
		SetGameStateInquire(game, stOneMsg);
	} else if (0 == strcasecmp(stOneMsg.m_szType, "blind")) {
		SetGameStateBlind(game, stOneMsg);
	} else {
		;
	}
	UpdateGameDeck(game);    
}

/*
 * Update the game's deck based on
 * the game's hand and community cards
 * game: the game to update
 */
void UpdateGameDeck(GameState *game)
{
    //Remove these cards from the deck
    memset(game->deck, 1, sizeof(game->deck) / sizeof(game->deck[0]));
    RemoveCardsFromDeck(game->deck, game->hand, game->handsize);
    RemoveCardsFromDeck(game->deck, game->community, game->communitysize);
}

/*
 * Print the current table information
 * game: the game state containing the table information
 * logfile: the file for logging output
 */
void PrintTableInfo(GameState *game, FILE *logfile)
{
    PrintCards(game, logfile);
    PrintPlayers(game, logfile);
    fprintf(logfile, "Current pot: %5d\n", game->current_pot);
}

/*
 * Print the AI's cards to the given file
 * game: the game state containing the cards
 * logfile: the file for logging output
 */
void PrintCards(GameState *game, FILE *logfile)
{
    fprintf(logfile, "Hand\tCommunity\n");

    //Print the hand
    if (game->handsize > 0)
    {
        fprintf(logfile, "%s", CARDS[game->hand[0]]);
        for (int i = 1; i < game->handsize; i++)
        {
            fprintf(logfile, " %s", CARDS[game->hand[i]]);
        }
    }
    else
    {
        fprintf(logfile, "\t");
    }
    fprintf(logfile, "\t");

    //Print the community cards
    if (game->communitysize > 0)
    {
        fprintf(logfile, "%s", CARDS[game->community[0]]);
        for (int i = 1; i < game->communitysize; i++)
        {
            fprintf(logfile, " %s", CARDS[game->community[i]]);
        }
    }
    fprintf(logfile, "\n");

}

/*
 * Print the current players at the table and their information
 * game: the game state containing the players
 * logfile: the file for logging output
 */
void PrintPlayers(GameState *game, FILE *logfile)
{
    Player *player;

    fprintf(logfile, "Me:\n");
    fprintf(logfile, "\tinitial_stack=%5d, call_amount=%5d\n", game->initial_stack, game->call_amount);
    fprintf(logfile, "\t        stack=%5d, current_bet=%5d\n", game->stack, game->current_bet);

    fprintf(logfile, "Opponents:\n");
    for (int i = 0; i < game->num_opponents; i++)
    {
        player = &game->opponents[i];
        if (player->folded)
        {
            fprintf(logfile, "[FOLDED] %10s: initial_stack=%5d, stack=%5d, current_bet=%5d\n", player->name, player->initial_stack, player->stack, player->current_bet);
        }
        else
        {
            fprintf(logfile, "         %10s: initial_stack=%5d, stack=%5d, current_bet=%5d\n", player->name, player->initial_stack, player->stack, player->current_bet);
        }
    }
}
