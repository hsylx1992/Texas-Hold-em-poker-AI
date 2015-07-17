#ifndef __POKER_AI_H__
#define __POKER_AI_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "action.h"
#include "eval.h"
#include "gamestate.h"
#include "timer.h"


// 输出
#define DEBUG_AI 		1
#define DEBUG_AGGRESS 	1

#define DEBUG_ONE_PAIRE 1

// 选择
#define UseMakeDecisionPreflop 	1
#define UseMakeDecisionflop 	1
/*
1 : org
2 : noPlayers-1
3 : good hand base org
*/
#define SIMU_ALG_TYPE			1

#define NUM_RAISE_LIMIT     5
#define SEED_COUNT          100


#define IsFold()	((ai->action.type==ACTION_FOLD)?1:0)
#define IsCall()	((ai->action.type==ACTION_CALL)?1:0)
#define IsBet()		((ai->action.type==ACTION_BET)?1:0)
#define IsCheck()	((ai->action.type==ACTION_CHECK)?1:0)
#define IsAllin()	((ai->action.type==ACTION_ALL_IN)?1:0)

#define SetFold()  	SetAction(ai,ACTION_FOLD,0)
#define SetCall()  	SetAction(ai,ACTION_CALL,0)
#define SetBet(c)  	SetAction(ai,ACTION_BET,myrand(1,5)+c)
#define SetCheck() 	SetAction(ai,ACTION_CHECK,0)
#define SetAllin() 	SetAction(ai,ACTION_ALL_IN,0)
#define AggSetBet(c) 	do {SetBet(MAX(c,ai->action.amount));} while(0)
#define AggSetCall() 	do {if(ai->action.type!=ACTION_ALL_IN && ai->action.type!=ACTION_BET){SetCall();}} while (0)
#define AggSetAllin()	SetAllin()


#define MAX(a,b)  (((a)>(b))?(a):(b))
#define MIN(a,b)  (((a)>(b))?(b):(a))


#if DEBUG_AGGRESS
#define DBASPRINTF(...) do  {	if(agg->level) {\
									printf("[%s>", agg->info_start);\
									printf(__VA_ARGS__);\
								}\
							}while(0);
#else
#define DBASPRINTF(...) 
#endif


typedef enum loglevel
{
    LOGLEVEL_NONE,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG
} LOGLEVEL;

typedef struct pokerai
{
    //Worker threads
    pthread_mutex_t mutex;
    pthread_t *threads;
    int num_threads;
    int timeout;

    //Random seeds for worker threads
    pthread_mutex_t seed_mutex;
    bool *seed_avail;
    int *seeds;

    //Scoring
    int games_won;
    int games_simulated;

    //Current game state
    GameState game;
    int num_times_raised;

    //Recommended action
    Action action;
    Action action_last;

    //Logging for debugging purposes
    LOGLEVEL loglevel;
    FILE *logfile;

	int aimoney;  //剩余的金币
    //Heuristics!
    //TODO
} PokerAI;

PokerAI *CreatePokerAI(int timeout);

void DestroyPokerAI(PokerAI *ai);

void SetLogging(PokerAI *ai, LOGLEVEL level, FILE *file);

void UpdateGameState(PokerAI *ai, stMsg_t stOneMsg);

void SetHand(PokerAI *ai, char **hand, int handsize);

void SetCommunity(PokerAI *ai, char **community, int communitysize);

bool MyTurn(PokerAI *ai);

char *GetBestAction(PokerAI *ai);

//double GetWinProbability(PokerAI *ai);

void WriteAction(PokerAI *ai, FILE *file);

int myrand(int a, int b);

void SetAction(PokerAI *ai, ActionType act, int amount);

void SetBetOrCall(PokerAI *ai, int a, int b, int amount);

void SetCallOrFold(PokerAI *ai, int a, int b, int amount);

void OprActionProtect(PokerAI *ai, int left_bet);

void OprBeforAgg(PokerAI *ai);

void RecordStat(PokerAI *ai);


#endif
