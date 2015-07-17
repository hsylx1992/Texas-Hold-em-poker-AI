#ifndef __POKERAI_AGGRESS_H__
#define __POKERAI_AGGRESS_H__

#include "pokerai.h"
#include "pokerai_preflop.h"


typedef struct Aggress__t {
	int my_pos;
	int beforeme_least_stack;
	int behindme_best_stack;
	int sum_other_stack;	
	int level;
	//int four_ths[5];	//0不用，1-4
	char info_start[10];
	int  type;	// 0 preflop; 3 flop
} Aggress_t;

typedef struct VarTran__t {
	int   naf;
	int   pv;
	float grp;
	float pwr;
	// ai->game 数据
	int noPlaying;
	int noPlayers;
	int call_no_bf;
	int fold_no_bf;
	int raise_no_bf;
	int allin_no_bf;
	int pos;
	PosType pos_type;
	int stack;
	int money;
	int current_bet;
	int call_amount;
	int raise_amount;
	int call_no;
	int raise_no;
	int raise_no_c;
	int round;
	int pot;
	int BB;				
	// adv usage var
	int noAfterme;
    double wp;
    double eg;
    int my_pos;
	int beforeme_least_stack;
	int behindme_best_stack;	
	int sum_other_stack;
	int same_stack;    
} VarTran_t;

void SetAggressStruct(Aggress_t *agg, int type);
void GetAggressLevel(Aggress_t *agg, PokerAI *ai, VarTran_t h);
void SetAggressAction(Aggress_t *agg, PokerAI *ai, VarTran_t h, int (*IsBetter)(int eval, int evalclass));


int GetPreflopValue(int *hand);

float GetPreflopGroup(int *hand);
float GetPreflopPower(int *hand);

float FileB_IsStrongHand(int *hand);
float FileB_IsSupperHand(int *hand);

int GetPreflopNoAfter(int *hand);

VarTran_t VarTran(PokerAI *ai);

void PrintDebugAI(PokerAI *ai, VarTran_t h, int flag, int type);

#endif
