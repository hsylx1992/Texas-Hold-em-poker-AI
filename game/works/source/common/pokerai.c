#include "pokerai.h"
#include "pokerai_simu.h"
#include "pokerai_preflop.h"
#include "pokerai_flop.h"
#include "pokerai_aggress.h"
//=========================================================================

PokerAI *CreatePokerAI(int timeout)
{
    PokerAI *ai = malloc(sizeof(*ai));
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);

    //Allocate worker thread members
    ai->num_threads = num_threads;
    ai->timeout = timeout;
    ai->threads = malloc(sizeof(pthread_t) * num_threads);
    pthread_mutex_init(&ai->mutex, NULL);

    //Create random seeds for the worker threads
    ai->seed_avail = malloc(sizeof(*ai->seeds) * num_threads);
    ai->seeds = malloc(sizeof(*ai->seeds) * num_threads);
    pthread_mutex_init(&ai->seed_mutex, NULL);
    for (int i = 0; i < num_threads; i++)
    {
        ai->seed_avail[i] = true;
        ai->seeds[i] = rand();
    }

    //Set the initial state to no other players
    ai->game.round_id = 0;
    
    ai->game.num_opponents = 0;
    ai->game.num_playing = 0;
    
	ai->game.initial_stack = 2000;		
	ai->game.money = 2000;
	ai->game.BB    = 40;
	ai->game.my3BB = 120;
		
    ai->logfile = NULL;
    return ai;
}

void DestroyPokerAI(PokerAI *ai)
{
    if (!ai) return;

    if (ai->logfile)
    {
        fclose(ai->logfile);
    }

    pthread_mutex_destroy(&ai->mutex);
    pthread_mutex_destroy(&ai->seed_mutex);

    free(ai->seeds);
    free(ai->threads);
    free(ai);
}

void SetLogging(PokerAI *ai, LOGLEVEL level, FILE *file)
{
    ai->loglevel = level;
    ai->logfile = file;
    fprintf(file, "Logging started\n");
    fprintf(file, "num_threads: %d\n", ai->num_threads);
    fprintf(file, "timeout: %dms\n\n", ai->timeout);
}

void UpdateGameState(PokerAI *ai, stMsg_t stOneMsg)
{
    ai->action.type = ACTION_UNSET;
    SetGameState(&ai->game, stOneMsg);

    if (0 == strcasecmp(stOneMsg.m_szType, "inquire")) {
        PrintTableInfo(&ai->game, ai->logfile);
    }
}

void SetHand(PokerAI *ai, char **hand, int handsize)
{
    ai->game.handsize = handsize;
    for (int i = 0; i < handsize; i++)
    {
        ai->game.hand[i] = StringToCard(hand[i]);
    }
}

void SetCommunity(PokerAI *ai, char **community, int communitysize)
{
    ai->game.communitysize = communitysize;
    for (int i = 0; i < communitysize; i++)
    {
        ai->game.community[i] = StringToCard(community[i]);
    }
}

bool MyTurn(PokerAI *ai)
{
    return ai->game.your_turn;
}

char *GetBestAction(PokerAI *ai)
{
    double winprob;
    double potodds;
    double expectedgain;
    ai->games_won = 0;
    ai->games_simulated = 0;
	
	//puts("GetBestAction");
    //Set the pot odds
    if (ai->game.call_amount > 0)
    { 
        potodds = (double) ai->game.call_amount / (ai->game.call_amount + ai->game.current_pot);
    }
    else
    {
        //There is nothing to be lost by calling
        potodds = 1.0 / ai->game.num_playing;
    }

	if (ai->game.communitysize == 0) {
		winprob = GetWinProbability(ai);
		expectedgain = winprob / potodds;
			
		MakeDecisionPreflop(ai);
        if (ai->loglevel >= LOGLEVEL_DEBUG)
        {
            fprintf(ai->logfile, "[my3BB:%d] [BB:%d]Huang preflop\n", ai->game.my3BB, ai->game.BB);
        }
		
	} else {
		//Set the rate of return
		winprob = GetWinProbability(ai);
		expectedgain = winprob / potodds;

		if (ai->loglevel >= LOGLEVEL_INFO)
		{
		    fprintf(ai->logfile, "Win probability: %.2lf%%\n", winprob * 100);
		    fprintf(ai->logfile, "Rate of return:  %.2lf\n", expectedgain);
		}

		ai->action.winprob = winprob;
		ai->action.expectedgain = expectedgain;

		//puts("before MakeDecisionFlop");
		MakeDecisionFlop(ai);
	}
	
    return ActionGetString(&ai->action);
}

void WriteAction(PokerAI *ai, FILE *file)
{
    switch(ai->action.type)
    {
    case ACTION_FOLD:
        fprintf(file, "ACTION:\tFOLDING\n");
        break;

    case ACTION_CALL:
        if (ai->action.bluff)
        {
            fprintf(file, "ACTION:\tCALLING (BLUFF)\n");
        }
        else
        {
            fprintf(file, "ACTION:\tCALLING\n");
        }
        break;

    case ACTION_BET:
        if (ai->action.bluff)
        {
            fprintf(file, "ACTION:\tBETTING %d (BLUFF)\n", ai->action.amount);
        }
        else
        {
            fprintf(file, "ACTION:\tBETTING %d\n", ai->action.amount);
        }
        break;

    default:
        fprintf(file, "!! No action set !!\n");
        break;
    }
}

//========================================================================================================================== 
int myrand(int a, int b) {
	if (a == b) return 0;
	if (a > b) {
		int t = a;
		a = b;
		b = t;
	}	
	return (rand()%(b-a) + a);
}
//==========================================================================================================================
void SetAction(PokerAI *ai, ActionType act, int amount) {
	switch (act) {
	case ACTION_FOLD:
		ai->action.type 	= ACTION_FOLD;
		ai->action.amount	= 0;
		break;
    case ACTION_CALL:
    	ai->action.type 	= ACTION_CALL;
    	ai->action.amount	= 0;
    	break;
    case ACTION_BET:
    	ai->action.type 	= ACTION_BET;
		//ai->action.bluff  = false;
		ai->action.amount 	= (int)(amount);     	
    	break;
    case ACTION_CHECK:
    	ai->action.type 	= ACTION_CHECK;
		ai->action.amount 	= 0;    	
    	break;
    case ACTION_ALL_IN:
    	ai->action.type 	= ACTION_ALL_IN;
    	ai->action.amount	= 99999;
    	break;
    default:
    	ai->action.type 	= ACTION_FOLD;
    	ai->action.amount	= 0;
    	break;
	}
}

void SetBetOrCall(PokerAI *ai, int a, int b, int amount) {
	int th = rand()%100;
	if (a+b == 0) {
		a = 50;
		b = 50;
	}
	a = 100*a/(a+b);
	b = 100-a;
	if (th < a) {
		SetBet(amount);
	} else {
		SetCall();
	}
}

void SetCallOrFold(PokerAI *ai, int a, int b, int amount) {
	int th 	= rand()%100;
	int isSB= ((ai->game.isSpecialPos == 5)?(1):(0));
	
	if (a+b == 0) {
		a = 50;
		b = 50;
	}	
	if (isSB) {
		//小盲注概率加成
		a = 2*a;
	}
	a = 100*a/(a+b);
	b = 100-a;	
	if (th < a) {
		SetCall();
	} else {
		SetFold();
	}
}
//==========================================================================================================================
void OprActionProtect(PokerAI *ai, int left_bet) {
	// bet保护
	if (IsBet()) {
		if (left_bet < ai->game.raise_amount) {
			SetCall();
			printf("--- raise ---> call --- \n");
		}
	}
	
	// call保护
	if (IsCall()) {
		if (left_bet < ai->game.call_amount) {
			SetFold();
			printf("--- call  ---> fold --- \n");
		}
	}
}

void OprBeforAgg(PokerAI *ai) {
	
    if ((ai->action.type == ACTION_FOLD) && (ai->game.call_amount == 0))
    {
    	printf("--- fold  ---> check --- \n");
        ai->action.type = ACTION_CALL;
    }

    //Don't get locked in a raise loop
    if ((ai->action.type == ACTION_BET) && (ai->game.raise_no > NUM_RAISE_LIMIT))
    {
    	printf("--- fold  ---> check --- (but should not here)\n");
        ai->action.type = ACTION_CALL;
    }	
}

#define RecordStatDebug 0

void RecordStat(PokerAI *ai) {
#if RecordStatDebug
    if (GetPreflopGroup(ai->game.hand) <= 2) {
		char action[20] = "";
		sprintf(action, "%s", ActionGetStringNoHuanHang(&(ai->action)));		
		printf("[%s]allin-%d,bet-%d,call-%d,check-%d,fold-%d\n", action, IsAllin(), IsBet(), IsCall(), IsCheck(), IsFold());
	}
#endif	
    // 目前没有ALL_IN
    if ((ai->action.type == ACTION_CALL) ||        
        (ai->action.type == ACTION_BET)  ||
        (ai->action.type == ACTION_ALL_IN) ){
    	ai->game.first_call = 0;
    }

    if (ai->action.type == ACTION_CALL) {
    	ai->game.call_no++;
    }
        
    if ((ai->action.type == ACTION_BET) 	||
    	(ai->action.type == ACTION_ALL_IN) 	){    	
    	if (ai->game.raise_no) {
    		ai->game.raise_no_c ++;
    	}
    	ai->game.raise_no++;
    } else {
    	ai->game.raise_no_c =0;
    }
#if RecordStatDebug    
    if (GetPreflopGroup(ai->game.hand) <= 2) {
		printf("bet-%d(%d),call-%d\n", ai->game.raise_no, ai->game.raise_no_c, ai->game.call_no);
	}
#endif 
	// 如果上次一次加注，这一次也需要加注，则连续加注计数器+1 raise_no_c
	// 否则清零    
}
