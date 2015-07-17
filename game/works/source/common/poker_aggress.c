#include "pokerai.h"
#include "pokerai_aggress.h"
#include <string.h>


//==========================================================================================================================
static
int power3(int n) {
	if (n <= 0) {
		return 1;
	}
	
	int ret = 1;
	while (n--) {
		ret = ret * 3;
	}
	return ret;
}

//==========================================================================================================================
VarTran_t VarTran(PokerAI *ai) {
	VarTran_t h;
	h.naf		= GetPreflopNoAfter(ai->game.hand);
	h.pv  		= GetPreflopValue(ai->game.hand);
	h.grp 		= GetPreflopGroup(ai->game.hand);
	h.pwr 		= GetPreflopPower(ai->game.hand);	
	// ai->game 数据
	h.noPlaying 	= ai->game.num_playing;
	h.noPlayers 	= ai->game.crt_player_no;
	h.call_no_bf    = ai->game.num_call_beforeme;
	h.fold_no_bf    = ai->game.num_fold_beforeme;
	h.raise_no_bf   = ai->game.num_raise_beforeme;
	h.allin_no_bf   = ai->game.num_allin_beforeme;
	h.pos			= ai->game.pos;
	h.pos_type		= ai->game.pos_type;
	h.stack			= ai->game.stack;
	h.money			= ai->game.money;
	h.current_bet 	= ai->game.current_bet;
	h.call_amount 	= ai->game.call_amount;
	h.raise_amount	= ai->game.raise_amount;
	h.call_no		= ai->game.call_no;
	h.raise_no		= ai->game.raise_no;
	h.raise_no_c	= ai->game.raise_no_c;
	h.round  		= ai->game.round_id;
	h.pot			= ai->game.current_pot;
	h.BB			= ai->game.BB;
	// adv usage var
	h.noAfterme		= h.noPlaying + h.fold_no_bf - h.pos;	
    h.wp 			= ai->action.winprob;
    h.eg			= ai->action.expectedgain;
    h.my_pos		= ai->game.my_pos;
	h.beforeme_least_stack  = ai->game.beforeme_least_stack;
	h.behindme_best_stack   = ai->game.behindme_best_stack;	
	h.sum_other_stack		= ai->game.sum_other_stack;
	h.same_stack	= ai->game.same_stack;    
	return h;
}


//==========================================================================================================================
void SetAggressStruct(Aggress_t *agg, int type) {
	agg->my_pos = 1;
	agg->type   = type;
	memset(agg->info_start, 0, sizeof(agg->info_start));
	if (type == 0) {
		//preflop
		memcpy(agg->info_start, "pre", sizeof("pre"));
	} else {
		//flop
		memcpy(agg->info_start, "", sizeof(""));
	}
}

#define SetIsAggress(level,str) do {\
							is_aggress=level;\
							DBASPRINTF(str);\
							}while(0)
							
void GetAggressLevel(Aggress_t *agg, PokerAI *ai, VarTran_t h) {
	// 排名最后 人数少 局数多 的时候，激进地争取名次
	int stack = h.money+h.stack;
	int round = h.round;
	int is_aggress = 0;	//agg->level
	
	/*
	int my_pos = 1;
	int beforeme_least_stack = h.stack+h.money;
	int behindme_best_stack  = 0;
	int sum_other_stack = 0;	
	int same_stack = 0;
			
	for (int i = 0; i < h.noPlayers-1; i++) {
		if (stack == ai->game.opponents[i].stack) {
			same_stack = 1;
		} else if (stack < ai->game.opponents[i].stack) {
			if (beforeme_least_stack > ai->game.opponents[i].stack) {
				beforeme_least_stack = ai->game.opponents[i].stack;
			}
			my_pos++;
		} else {
			if (behindme_best_stack  < ai->game.opponents[i].stack) {
				behindme_best_stack  = ai->game.opponents[i].stack;
			}				
		}
		sum_other_stack += ai->game.opponents[i].stack;
	}
	*/
	int my_pos 				 = h.my_pos;
	int beforeme_least_stack = h.beforeme_least_stack;
	int behindme_best_stack  = h.behindme_best_stack;
	int sum_other_stack 	 = h.sum_other_stack;	
	int same_stack 		 	 = h.same_stack;	
	///////// 人少的时候会被盲注扣死的  //~~~~~~
	if ((my_pos >  2) &&
		(round  > 50) && 
		(600 - round > (int)(1.5*4000*h.noPlayers/h.BB/1.5))	){
		SetIsAggress(1, "AS520");
	}
	/////////
		
	if (round >= 400 && h.noPlayers <= 3) {		
		if (same_stack) {
			is_aggress = 0;
		} else {
			if (my_pos == 1) {
				is_aggress = 0;
			} else if (my_pos == h.noPlayers) {
				SetIsAggress(1, "AS100");
			} else if ((1.5*behindme_best_stack < stack) && (1.5*stack < beforeme_least_stack)) {
				SetIsAggress(1, "AS100");
			} else {
				is_aggress = 0;
			}
		}
		if (my_pos == 1) {
			is_aggress = 0;
		}		
	}
		
	if (round >= 400 && my_pos != 1) {
		if ((1.5*behindme_best_stack < stack) 	&& 
			(1.5*stack < beforeme_least_stack)) {		
			is_aggress = 1;
			//printf("\n!!!!!%f < %d && %d < %f!!!!!\n", 1.5*behindme_best_stack, stack, stack, 1.5*beforeme_least_stack);
			SetIsAggress(1, "AS200");
		}
	}
				
	// 排名很后的时候，更加激进地争取名次
	int daoshudiji = (h.noPlayers - my_pos + 1);
	
	if (my_pos > 2) {
		
		if (daoshudiji == 4) {			
			if (round >= 550) {
				is_aggress = 3;
			} else if (round >= 500) {
				is_aggress = 2;
			} else if (round >= 400) {
				is_aggress = 1;
			}			
		}
				
		if (daoshudiji == 2 || daoshudiji == 3) {			
			if (round >= 500) {
				SetIsAggress(3, "AS320");
			} else if (round >= 450) {
				SetIsAggress(2, "AS320");
			} else if (round >= 350) {
				SetIsAggress(1, "AS320");
			}
		}
					
		if (daoshudiji == 1) {			
			if (round >= 450) {
				SetIsAggress(3, "AS330");
			} else if (round >= 400) {
				SetIsAggress(2, "AS330");
			} else if (round >= 300) {
				SetIsAggress(1, "AS330");
			}
			if (stack < 10*40) {
				SetIsAggress(2, "AS330");
			}
		}			
	} // END排名很后的时候，更加激进地争取名次
	
	// 只有两个人的时候，更加激进地争取名次
	if (ai->game.crt_player_no == 2 && my_pos == 2) {
		if (stack < ai->game.opponents[0].stack) {			
			SetIsAggress(3, "AS400");
		}
	}

	//~~~~~~~~~~~~~~
	if (is_aggress) {
		if (my_pos == h.noPlayers) {
			if (!(1.5*behindme_best_stack < stack)) {
				is_aggress --;
				DBASPRINTF("[->%d]", is_aggress);
			}		
		} else if (my_pos ==  1) {
			if (!(1.5*behindme_best_stack < stack)) {
				is_aggress --;
				DBASPRINTF("[->%d]", is_aggress);
			}		
		} else {															 
			if (! ( (1.5*behindme_best_stack < stack)  && 
					(1.5*stack < beforeme_least_stack) ) ) {
				is_aggress --;
				DBASPRINTF("[->%d]", is_aggress);
			}
		}
		DBASPRINTF("\n");
	}
	
	agg->my_pos = my_pos;
	agg->beforeme_least_stack 	= beforeme_least_stack;
	agg->behindme_best_stack  	= behindme_best_stack;
	agg->sum_other_stack 		= sum_other_stack;	
	agg->level	= is_aggress;

}

/*
SetAggressStruct(&agg, 0);
GetAggressLevel(&agg, ai, h);
SetAggressAction(&agg, ai, h, IsBetterClassPreflopPwr);
*/

void SetAggressAction(Aggress_t *agg, PokerAI *ai, VarTran_t h, int (*IsBetter)(int eval, int evalclass)) {

	int stack = h.stack;
	int round = h.round;
	//int pwr   = h.pwr;
	//int pv    = h.pv;
	
	int eval  = h.pwr;
	if (agg->type == 1) {
		eval = h.pwr;
	}
	
	int my_pos 				 = agg->my_pos;
	int beforeme_least_stack = agg->beforeme_least_stack;
	int behindme_best_stack  = agg->behindme_best_stack;
	int sum_other_stack      = agg->sum_other_stack;	
	int is_aggress			 = agg->level;
		
	//~~~~~~~~~~~~~
	int aggbet = MAX(rand()%80, rand()%(is_aggress*stack/10+1));
	//int call_no = ai->game.call_no;
	if (is_aggress 															&& 
		(ai->action.type == ACTION_FOLD||ai->action.type == ACTION_CHECK) 	){
		ai->game.call_no++;
	}
	if (is_aggress == 3) {
		DBASPRINTF("[%4d]===[in-%d-of-%d-lv-%d]", round, my_pos, h.noPlayers, is_aggress);
		if (IsBetter(eval, 3)) {
			aggbet = power3(h.call_no);
			DBASPRINTF("---call %d", h.call_amount);
			AggSetCall();
		} else if (IsBetter(eval, 4)) {
			DBASPRINTF("---raise %d", 4*aggbet);
			AggSetBet(4*aggbet);			
		} else if (h.pv >= 12) {
			DBASPRINTF("---raise %d", 2*aggbet);
			AggSetBet(2*aggbet);
		} else {
			DBASPRINTF("---call %d", h.call_amount);
			AggSetCall();			
		}
		DBASPRINTF("\n");		
	} if (is_aggress == 2) {
		DBASPRINTF("[%4d]===[in-%d-of-%d-lv-%d]", round, my_pos, h.noPlayers, is_aggress);
		if (IsBetter(eval, 3)) {
			DBASPRINTF("---raise %d", 2*aggbet);
			AggSetBet(2*aggbet);		
		} else if (IsBetter(eval, 4)) {
			DBASPRINTF("---raise %d", 2*aggbet);
			AggSetBet(2*aggbet);
		}
		DBASPRINTF("\n");			
	} else if (is_aggress == 1){
		DBASPRINTF("[%4d]===[in-%d-of-%d-lv-%d-%d<<%d<<%d]", round, my_pos, h.noPlayers, is_aggress, behindme_best_stack, stack, beforeme_least_stack);
		if (IsBetter(eval, 2)) {
			aggbet = MIN(aggbet*2, rand()%(ai->game.raise_no*behindme_best_stack/2+1));
			DBASPRINTF("---raise %d", aggbet);
			AggSetBet(aggbet);
		} else if (IsBetter(eval, 3)) {
			aggbet = MIN(aggbet*2, rand()%(ai->game.raise_no*behindme_best_stack/4+1));
			DBASPRINTF("---raise %d", aggbet);
			AggSetBet(aggbet);
		} else if (IsBetter(eval, 4)) {
			DBASPRINTF("---raise %d", aggbet);
			AggSetBet(aggbet);
		}
		DBASPRINTF("\n");
	} // END 激进地争取名次
	
	// 排第一的时候，如果对方没有机会赢，一直fold
	int all_fold_lost_money = (600-round)*(1.5*h.BB)/h.noPlayers;
	if (my_pos == 1 && (stack-sum_other_stack-all_fold_lost_money>0) ) {		
		if (IsBet() || IsAllin()) {SetCall();}
		if (IsCall()) {SetFold();}
		printf("--- fold till end --- no.1 ---\n");
	}	
}


//=============================================================================
//naf : number of people after me
int nafDuizi[13] = { 2,  3,  4,  5,  6,  7,  8,  9,  9,  9, 10, 10, 10};
//				    22  33  44                                  KK  AA

int nafTonghua[13][12] = 	{ {0},
							  {0},
							  {0, 0},
							  {0, 0, 0},
							  {0, 0, 0, 0},
							  {0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 1, 2},
							  {0, 0, 0, 0, 0, 0, 0, 0, 1, 2},
							  {0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 4},
							  {3, 3, 3, 3, 3, 3, 3, 4, 5, 6,10,10}
							 };
int nafBuTonghua[13][12] = 	{ {0},
							  {0},
							  {0, 0},
							  {0, 0, 0},
							  {0, 0, 0, 0},
							  {0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 9}
							 };
int GetPreflopNoAfter(int *hand)
{
	int c1, c2;
	int naf = 0;
	
	if (hand[0]>hand[1]) {
		c1 = hand[0];
		c2 = hand[1];
	} else {
		c1 = hand[1];
		c2 = hand[0];
	}

    int c1val  = (c1 - 1) / 4;
    int c2val  = (c2 - 1) / 4;
    int c1suit = (c1 - 1) % 4;
    int c2suit = (c2 - 1) % 4;    

    if (c1val == c2val) {
        naf = nafDuizi[c1val];
    } else if (c1suit == c2suit) {
        naf = nafTonghua[c1val][c2val];
    } else {
    	naf = nafBuTonghua[c1val][c2val];
    }
    
    return naf;
}

//=============================================================================

//==========================================================================================================================
//2 3 4  5  6  7  8  9  T  J  Q  K  A
//1 5 9 13 17 21 25 29 33 37 41 45 49
//0 1 2  3  4  5  6  7  8  9 10 11 12
#define to2 0
#define to3 1
#define to4 2
#define to5 3
#define to6 4
#define to7 5
#define to8 6
#define to9 7
#define toT 8
#define toJ 9
#define toQ 10
#define toK 11
#define toA 12
//FileB 网上关于无上限的那文章
float FileB_IsSupperHand(int *hand)
{
	int ret = 0;
	int c1, c2;
	
	if (hand[0]>hand[1]) {
		c1 = hand[0];
		c2 = hand[1];
	} else {
		c1 = hand[1];
		c2 = hand[0];
	}

    int c1val  = (c1 - 1) / 4;
    int c2val  = (c2 - 1) / 4;
    int c1suit = (c1 - 1) % 4;
    int c2suit = (c2 - 1) % 4;    
	
	if (c1val == c2val && c1val >= toQ) {
		// AA KK QQ
		ret = 1;
	}
	
	if (c1suit == c2suit) {
		//AKs AQs
        if (c1val == toA && c2val >= toQ) {
        	ret = 1;	
        }
    }
    
    return ret;
}
float FileB_IsStrongHand(int *hand)
{
	int ret = 0;
	int c1, c2;
	
	if (hand[0]>hand[1]) {
		c1 = hand[0];
		c2 = hand[1];
	} else {
		c1 = hand[1];
		c2 = hand[0];
	}

    int c1val  = (c1 - 1) / 4;
    int c2val  = (c2 - 1) / 4;
    int c1suit = (c1 - 1) % 4;
    int c2suit = (c2 - 1) % 4;    
	
	if (c1val == c2val) {
		// AA KK QQ ... 22
		ret = 1;
	}
	
	if (c1suit == c2suit) {
		// AKs AQs AJs KQs
        if ((c1val == toA && c2val >= toJ) ||
        	(c1val == toK && c2val == toQ) ) {
        	ret = 1;	
        }
    } else {
    	// AK AQ KQ
        if ((c1val == toA && c2val >= toQ) ||
        	(c1val == toK && c2val == toQ) ) {
        	ret = 1;	
        }    	
    }
    
    return ret;
}

//==========================================================================================================================
//int pwGroupD[13] = {7, 7, 7, 6, 6, 5, 4, 3, 2, 1, 1, 1, 1};
float pwGroupD[13] = {3.4, 3.4, 3.4, 2.4, 2.4, 2.4, 2.2, 2.2, 2, 1, 1, 1, 1};
float pwGroupS[13][12] = 	{ {7},
							  {7},
							  {7, 7},
							  {9, 7, 6},
							  {9, 9, 7, 5},
							  {9, 9, 7, 6, 5},
							  {9, 9, 9, 7, 6, 5},
							  {9, 9, 9, 9, 7, 5, 4},
							  {9, 9, 9, 9, 9, 7, 5, 4},
							  {9, 9, 9, 9, 9, 7, 6, 4, 3},
							  {9, 9, 9, 9, 9, 9, 7, 5, 4, 3},
							  {7, 7, 7, 7, 7, 7, 7, 6, 4, 3, 2},
							  {5, 5, 5, 5, 5, 5, 5, 5, 3, 2, 2, 1}
							 };
float pwGroupO[13][12] = 	{ {7},
							  {9},
							  {9, 9},
							  {9, 9, 7},
							  {9, 9, 9, 7},
							  {9, 9, 9, 9, 7},
							  {9, 9, 9, 9, 9, 7},
							  {9, 9, 9, 9, 9, 9, 7},
							  {9, 9, 9, 9, 9, 9, 7, 7},
							  {9, 9, 9, 9, 9, 9, 7, 7, 5},
							  {9, 9, 9, 9, 9, 9, 9, 7, 6, 5},
							  {9, 9, 9, 9, 9, 9, 9, 7, 6, 5, 4},
							  {9, 9, 9, 9, 9, 9, 9, 7, 6, 4, 3, 2}
							 };
							 
float GetPreflopGroup(int *hand)
{
	int c1, c2;
	float grp = 9.0;
	
	if (hand[0]>hand[1]) {
		c1 = hand[0];
		c2 = hand[1];
	} else {
		c1 = hand[1];
		c2 = hand[0];
	}

    int c1val  = (c1 - 1) / 4;
    int c2val  = (c2 - 1) / 4;
    int c1suit = (c1 - 1) % 4;
    int c2suit = (c2 - 1) % 4;    

    if (c1val == c2val) {
        grp = pwGroupD[c1val];
    } else if (c1suit == c2suit) {
        grp = pwGroupS[c1val][c2val];
    } else {
    	grp = pwGroupO[c1val][c2val];
    }    

    return (float)grp;
}

float GetPreflopPower(int *hand)
{
	float grp = GetPreflopGroup(hand);
	float pwr = 10-grp;
	return (float)pwr;
}
//==========================================================================================================================
//pv : preflop value
int pvDuizi[13] = {11, 11, 11, 12, 13, 13, 15, 16, 19, 22, 27, 30, 40};
//				   22  33  44                                  KK  AA
//pvDuizi[1] - pvDuizi[13] 代表 AA 22 到 KK

//0 : 有值，在对角
//-1 : 成对
int pvTonghua[13][12] = 	{ {0},
							  {7},
							  {7, 8},
							  {8, 9,10},
							  {6, 8,10,11},
							  {6, 7, 9,10, 11},
							  {6, 6, 8, 9, 11, 12},
							  {7, 7, 7, 8, 10, 12, 13},
							  {8, 8, 8, 8, 10, 11, 13, 15},
							  {8, 9, 9, 9,  9, 11, 13, 15, 18},
							  {9,10,10,10, 11, 11, 13, 15, 18, 19},
							  {11,11,11,12, 12, 13, 13, 15, 18, 20, 21},
							  {13,14,14,15, 14, 14, 15, 16, 19, 20, 22, 24}
							 };
int pvBuTonghua[13][12] = 	{ {0},
							  {1},
							  {2, 3},
							  {2, 4, 5},
							  {1, 3, 4, 6},
							  {0, 1, 3, 5,  6},
							  {0, 1, 2, 4,  5,  7},
							  {1, 1, 1, 3,  4,  6,  8},
							  {2, 2, 2, 2,  4,  6,  8, 10},
							  {2, 2, 3, 3,  3,  5,  7,  9, 13},
							  {3, 3, 4, 4,  4,  5,  7,  9, 12, 14},
							  {4, 5, 5, 5,  6,  6,  7,  9, 13, 14, 16},
							  {7, 7, 8, 8,  7,  8,  9, 10, 13, 14, 16, 19}
							 };
										
int GetPreflopValue(int *hand)
{
	int c1, c2;
	int pv = 0;
	
	if (hand[0]>hand[1]) {
		c1 = hand[0];
		c2 = hand[1];
	} else {
		c1 = hand[1];
		c2 = hand[0];
	}

    int c1val  = (c1 - 1) / 4;
    int c2val  = (c2 - 1) / 4;
    int c1suit = (c1 - 1) % 4;
    int c2suit = (c2 - 1) % 4;    

    if (c1val == c2val) {
        pv = pvDuizi[c1val];
    } else if (c1suit == c2suit) {
        pv = pvTonghua[c1val][c2val];
    } else {
    	pv = pvBuTonghua[c1val][c2val];
    }
    
    return pv;
}


//=============================================================================

void PrintDebugAI(PokerAI *ai, VarTran_t h, int flag, int type) {
#if DEBUG_AI
	
	h = VarTran(ai);

	char *CARDS[] = {"XX",                                                  \
	"2S", "2C", "2D", "2H", "3S", "3C", "3D", "3H", "4S", "4C", "4D", "4H", \
	"5S", "5C", "5D", "5H", "6S", "6C", "6D", "6H", "7S", "7C", "7D", "7H", \
	"8S", "8C", "8D", "8H", "9S", "9C", "9D", "9H", "TS", "TC", "TD", "TH", \
	"JS", "JC", "JD", "JH", "QS", "QC", "QD", "QH", "KS", "KC", "KD", "KH", \
	"AS", "AC", "AD", "AH"                                                  \
	};
	char action[20] = "";
	int  actionamount= ai->action.amount;
	int  mynewpot = 0;
	sprintf(action, "%s", ActionGetStringNoHuanHang(&(ai->action)));
	
	//if (h.grp <= 7.0) {
	if (flag || h.round == 600 || h.stack+h.money<=3*h.BB) {
		printf("{%dof%d|%d|%3dr}", h.my_pos, h.noPlayers, h.noPlaying, h.round);
		
		if (type == 0) {
			//preflop
			printf("{ pv:%2d          }", h.pv);
		} else {
			//flop
			printf("{ wp:%4.2f|eg:%4.1f}", h.wp, h.eg);
		}
		
		printf("{pos%d-%d|act:%dc,%dr,%drc}", h.pos, h.pos_type, h.call_no, h.raise_no, h.raise_no_c);
		
		printf("{%.15s:", action);
		if (IsFold()) {
			mynewpot = h.current_bet;
			printf("|%4dcXX>%4d", h.call_amount, mynewpot);
		}
		if (IsCall()) {
			mynewpot = h.call_amount+h.current_bet;
			printf("|%4dc-->%4d", h.call_amount, mynewpot);
		}
		if (IsCheck()){
			mynewpot = h.current_bet;
			printf("|%4db==>%4d", 0, mynewpot);
		}
		if (IsBet()){
			mynewpot = actionamount+h.call_amount+h.current_bet;
			printf("|%4db++>%4d", h.raise_amount, mynewpot);
		}		
		if (IsAllin()) {
			mynewpot = h.stack+h.current_bet;
			printf("|%4db!!>%4d", h.raise_amount, mynewpot);		
		}
		printf("}");
		
		printf("-[pot:%4d|sk%5d|c_m%4d|r_m%4d]", h.pot, h.stack, h.call_amount, h.raise_amount);
		
		printf("-[%2s %2s ", CARDS[ai->game.hand[0]], CARDS[ai->game.hand[1]]);
		
		if (type == 0) {
			//preflop
			
		} else {
			//flop
			//printf("| ");
			for (int i = 0; i < ai->game.communitysize; i++) {
				printf("%2s ", CARDS[ai->game.community[i]]);
			}			
		}
		printf("\b]\n");
	}	
#endif	
}

//=============================================================================
