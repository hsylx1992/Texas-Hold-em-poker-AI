#include "pokerai.h"
#include "pokerai_aggress.h"

#include "eval.h"

//#include "pokerai_flop.h"


static int last_communitysize = 0;
static int anew_communitysize = 0;

extern int last_round_id;
extern int anew_round;
// 配置
//int cfClassStragetRaiseForStart_len = 3;	//三级 [1] [2] [3]
//int cfClassStragetRaiseForStart[] = {-1, 1, 1, 6, -1};

static int g_maxBet;
static int g_crtBet;
static int g_betno;

//static int is_print_one_time_info = 1;

#if DEBUG_ONE_PAIRE
#define DBONEPAIRE(...) do  {	printf(__VA_ARGS__);\
							}while(0);
#else
#define DBONEPAIRE(...) 
#endif

//==========================================================================================================================

static
int GetFlopStraget(int round) {
	
	int stg = 1;
	if (round <= myrand(90,100)) {
		stg = 1;
	} else {
		stg = -1;
	}
	return stg;
}

//==========================================================================================================================
#if 0
static 
int IsBetterClassFlopWinprob(int pwr, int class) {	
	int aggWinprobClass[5]   = {100, 8, 7.7, 7, 6};
	if (pwr >= aggWinprobClass[class]) {
		return 1;
	} else {
		return 0;
	}
}
#endif
/*
static
int StackProtection(PokerAI *ai, VarTran_t h, int wpBet) {
}
*/

#define DEBUG_GetCardType 0
static
int GetCardType(PokerAI *ai, VarTran_t h) {
#if DEBUG_GetCardType
	char *CARDS[] = {"XX",                                                  \
	"2S", "2C", "2D", "2H", "3S", "3C", "3D", "3H", "4S", "4C", "4D", "4H", \
	"5S", "5C", "5D", "5H", "6S", "6C", "6D", "6H", "7S", "7C", "7D", "7H", \
	"8S", "8C", "8D", "8H", "9S", "9C", "9D", "9H", "TS", "TC", "TD", "TH", \
	"JS", "JC", "JD", "JH", "QS", "QC", "QD", "QH", "KS", "KC", "KD", "KH", \
	"AS", "AC", "AD", "AH"                                                  \
	};
#endif	
	int n = 2 + ai->game.communitysize;
	// =============
	if (n < 5) {
		// ERROR
		return 10;
	} else if (n == 5) {
	
		int me[5] = {0};
		for (int i = 0; i < 2; i++)
		{
		    me[i] = ai->game.hand[i];		    
		}
		for (int i = 2; i < 5; i++)
		{
		    me[i] = ai->game.community[i - 2];		    
		}		
#if DEBUG_GetCardType		
		int meprint[5] = {0};
	

		for (int i = 0; i < 2; i++)
		{
		    meprint[i] = ai->game.hand[i];
		}
		for (int i = 2; i < 5; i++)
		{
		    meprint[i] = ai->game.community[i - 2];
		}
#endif					
		short cardvale = eval_hand_hsy(me, 5);
		short cardtype = hand_rank(cardvale);
#if DEBUG_GetCardType	
		printf("\n\n");

		for (int i = 0; i < 5; i++)
		{
		    printf("%2s ", CARDS[meprint[i]]);        
		}
		printf("\n -- card vale %5d --- \n", cardvale);
#endif		
		return cardtype;				
	} else if (n == 6) {
	
		int me[6] = {0};
		for (int i = 0; i < 2; i++)
		{
		    me[i] = ai->game.hand[i];		    
		}
		for (int i = 2; i < 6; i++)
		{
		    me[i] = ai->game.community[i - 2];		    
		}		
#if DEBUG_GetCardType		
		int meprint[6] = {0};
	

		for (int i = 0; i < 2; i++)
		{
		    meprint[i] = ai->game.hand[i];
		}
		for (int i = 2; i < 6; i++)
		{
		    meprint[i] = ai->game.community[i - 2];
		}
#endif					
		short cardvale = eval_hand_hsy(me, 6);
		short cardtype = hand_rank(cardvale);
#if DEBUG_GetCardType	
		printf("\n\n");

		for (int i = 0; i < 6; i++)
		{
		    printf("%2s ", CARDS[meprint[i]]);        
		}
		printf("\n -- card vale %5d --- \n", cardvale);
#endif		
		return cardtype;				
	} else if (n == 7) {
	
		int me[7] = {0};
		for (int i = 0; i < 2; i++)
		{
		    me[i] = ai->game.hand[i];		    
		}
		for (int i = 2; i < 7; i++)
		{
		    me[i] = ai->game.community[i - 2];		    
		}		
#if DEBUG_GetCardType		
		int meprint[7] = {0};
	

		for (int i = 0; i < 2; i++)
		{
		    meprint[i] = ai->game.hand[i];
		}
		for (int i = 2; i < 7; i++)
		{
		    meprint[i] = ai->game.community[i - 2];
		}
#endif					
		short cardvale = eval_hand_hsy(me, 7);
		short cardtype = hand_rank(cardvale);
#if DEBUG_GetCardType	
		printf("\n\n");

		for (int i = 0; i < 7; i++)
		{
		    printf("%2s ", CARDS[meprint[i]]);        
		}
		printf("\n -- card vale %5d --- \n", cardvale);
#endif		
		return cardtype;				
	} else {
		// ERROR
		return 10;
	}				
	// =============
}

static
void PrintCardType(int type) {
	char *INFO[] = {"XX",
	"1同花顺",
	"2四条",
	"3三带二",
	"4同花",
	"5顺子",
	"6三条",
	"7两对",
	"8一对",
	"9高牌"
	};
	if (type <= 9) {
		printf("\n --- %6s --- \n", INFO[type]);
	} else {
		printf("\n --- card num ERROR --- \n");
	}
}

#if 1

static
int GetVal(int card) {
	return ((card - 1) % 4);
}

static
int GetSuit(int card) {
	return ((card - 1) / 4);
}

static
int CheckMaxCard(int *card, int n) {
	int max = card[0];
	for (int i = 0; i < n; i++) {
		if (max < card[i]) {
			max = card[i];
		}
	}
	return max;
}
static
int CheckSecondCard(int *card, int n, int max) {
	int second = -1;
	for (int i = 0; i < n; i++) {
		if (max != card[i]) {
			if (second < card[i]) {
				second = card[i];
			}
		}
	}
	return second;
}
/*
static
int CheckMinCard(int *card, int n) {
	int min = card[0];
	for (int i = 0; i < n; i++) {
		if (min > card[i]) {
			min = card[i];
		}
	}
	return min;
}
*/
#if 0
static
int GetTwoPair() {
	;
}
#endif
static
int GetOnePair(int *me, int *flop) {
	int maxpairval = -1;
	int maxpair    = -1;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			int c1 = me[i];
			int c2 = flop[j];
			int c1val  = (c1 - 1) / 4;
			int c2val  = (c2 - 1) / 4;
			if (c1val == c2val) {
				if (maxpairval < c1val) {
					maxpairval = c1val;
					maxpair    = c1;
				}
			}			
		}
	}
	/*
	int maxpair = -1;
	for (int i = 0; i < n; i++) {
		for (int j = i+1; j < n; j++) {
			int c1 = card[i];
			int c2 = card[j];
			int c1val  = (c1 - 1) / 4;
			int c2val  = (c2 - 1) / 4;
			if (c1val == c2val) {
				if (maxpair < c1val) {
					maxpair = c1val;
				}
			}
		}
	}
	*/
	return maxpair;
}

static
int IsMyHandPair(int *card) {
		int c1 = card[0];
		int c2 = card[1];
		int c1val  = (c1 - 1) / 4;
		int c2val  = (c2 - 1) / 4;
		return ((c1val == c2val)?(1):(0));
}

static
int IsMyHandShunzi(int *card) {
	int c1 = card[0];
	int c2 = card[1];
	if (c1 < c2) {int t = c1;c1=c2;c2=t;}
	int c1val = (c1 - 1) % 4;
	int c2val = (c2 - 1) % 4;
	return ((c1val - c2val == 1)?(1):(0));	
}
static
int IsFourShunzi(int *me, int *flop) {
	//int n = 5;
	if (IsMyHandShunzi(me) == 0) {return 0;}
	int c1 = me[0];
	int c2 = me[1];
	if (c1 < c2) {int t = c1;c1=c2;c2=t;}
	int c1val = (c1 - 1) % 4;
	int c2val = (c2 - 1) % 4;
	int c0val = c1val - 1;
	int c3val = c2val + 1;
	int flag0 = 0;
	int flag3 = 0;
	for (int i = 0; i < 3; i++){
		if (c0val == flop[i]) {flag0 = 1;}
		if (c3val == flop[i]) {flag3 = 1;}
	}
	if (flag0 && flag3) {
		return 1;
	} else {
		return 0;
	}
		
}
static
int IsMyHandTonghua(int *card) {
	int c1 = card[0];
	int c2 = card[1];
	int c1suit = (c1 - 1) % 4;
	int c2suit = (c2 - 1) % 4;
	return ((c1suit == c2suit)?(1):(0));
}

static
int IsFourTonghua(int *me, int *flop) {
	//int n = 5;
	if (IsMyHandTonghua(me) == 0) {return 0;}
	int suit1 = (me[0] - 1) % 4;
	int cnt   = 0;
	for (int i = 0; i < 3; i++) {
		int suit2 = (flop[i] - 1) % 4; 
		if (suit2 != suit1) {cnt++;}
		if (cnt >= 2) {return 0;}
	}
	
	return 1;
}

static
int IsFlopTonghua(int *flop) {
	int c1 = flop[0];
	int c2 = flop[1];
	int c3 = flop[2];
	int c1suit = (c1 - 1) % 4;
	int c2suit = (c2 - 1) % 4;
	int c3suit = (c3 - 1) % 4;
	if (c1suit == c2suit && c1suit == c3suit) {
		return 1;
	} else {
		return 0;
	}
}

static
int IsFlopShunzi(int *flop) {
	int c1 = flop[0];
	int c2 = flop[1];
	int c3 = flop[2];
	int c[] = {c1,c2,c3};
	for (int i = 0; i < 3; i++) {
		for (int j = i; j < 3; j++) {
			if (c[i] < c[j]) {
				int t = c[i];
				c[i] = c[j];
				c[j] = t;
			}
		}
	}
	int c1val = (c[0] - 1) % 4;
	int c2val = (c[1] - 1) % 4;
	int c3val = (c[2] - 1) % 4;
	if (c1val - c2val == 1 && c2val - c3val == 1) {
		return 1;
	} else {
		return 0;
	}
}

static
int IsFlopGuilv(int *flop) {
	if (IsFlopTonghua(flop) || IsFlopShunzi(flop)) {return 1;}
	return 0;
}

//==========================================================================================================================
//2 3 4  5  6  7  8  9  T  J  Q  K  A
//1 5 9 13 17 21 25 29 33 37 41 45 49
//0 1 2  3  4  5  6  7  8  9 10 11 12

// 3张牌的时候!!!必须是三张牌

/*
 0 fold
 1 call	
 2 bet 1 次
 */
typedef struct FlopAction__t {
	int use;			// 是否特殊处理
	int type;			// 动作
	int amount;			// 下注值
	int raise_times;	// 可进行的加注次数
} FlopAction_t;

FlopAction_t g_FlopAction;

static
void FAClear() {
	g_FlopAction.use = 0;
	g_FlopAction.type = 0;
	g_FlopAction.amount = 0;
	g_FlopAction.raise_times = 0;
}
static
void FAFold() {
	g_FlopAction.use = 1;
	g_FlopAction.type = 0;
	g_FlopAction.amount = 0;
	g_FlopAction.raise_times = 0;	
}

static
void FABet(int amount, int times) {
	g_FlopAction.use = 1;
	g_FlopAction.type = 2;
	g_FlopAction.amount = amount;
	g_FlopAction.raise_times = times;	
}

static
void FAUpdateFinish() {
	if (g_FlopAction.use == 1) {
		if (g_FlopAction.type == 2 && g_FlopAction.raise_times == 0) {
			g_FlopAction.use = 0;
		}
	}
	if (g_FlopAction.use == 0) {
		FAClear();
	}
}
static
void FAUpdateBet(PokerAI *ai) {
	if (IsBet()) {
		if (g_FlopAction.raise_times) {
			g_FlopAction.raise_times -= 1;
		} else {
			FAUpdateFinish();
		}
	}
}

// 确定是一对手牌
static
void commonCheckOnePair(PokerAI *ai, VarTran_t h) {
	int hand[2] = {0};
	int flop[3] = {0};
	//int all[5]  = {0};
	for (int i = 0; i < 2; i++)
	{
	    hand[i] = ai->game.hand[i];
	    //all[i]  = ai->game.hand[i];
	}
	for (int i = 0; i < 3; i++)
	{
	    flop[i] = ai->game.community[i];
	    //all[i+2]  = ai->game.community[i + 2]; 
	}
	
	int hand_max = CheckMaxCard(hand, 2);
	//int hand_min = CheckMinCard(hand, 2);
	int flop_max = CheckMaxCard(flop, 3);
	int flop_sec = CheckSecondCard(flop, 3, flop_max);
	
	if (IsMyHandPair(hand)) {
		//int hand_pair = GetOnePair(hand, flop);
		DBONEPAIRE("hand-pair(%d) pair(%d) flop(%d %d)\n", GetVal(hand[0]), GetVal(hand[0]), GetVal(flop_max), GetVal(flop_sec));
		if (GetVal(flop_max) <= GetVal(hand_max)) {
			//(6)
			//"加注 不让对手看转牌"
			FABet(h.stack/2, 10);			//!!!动态
			DBONEPAIRE("6-bet 1\n");
		} else if (GetVal(flop_sec) <= GetVal(hand_max) && GetVal(hand_max) < GetVal(flop_max)) {
			//(9)
			//"别人让拍，加注一次"
			//"否则弃牌"
			if (h.call_amount == 0) {
				FABet(h.stack/4, 1);		//!!!加注一次
				DBONEPAIRE("9-bet 1\n");
			} else {
				FAFold();
				DBONEPAIRE("9-fold\n");
			}
		} else {
			//(12)
			//"马上弃牌"
			FAFold();
			DBONEPAIRE("12-fold\n");
		}
	} else {
		int hand_defend = hand[0];
		int hand_pair = GetOnePair(hand, flop);
		if (hand_defend == hand_pair) {
			hand_defend = hand[1];
		}
		DBONEPAIRE("hand-defend(%d) pair(%d) flop(%d %d)\n", GetVal(hand_defend), GetVal(hand_pair), GetVal(flop_max), GetVal(flop_sec));
		if (GetVal(flop_max) <= GetVal(hand_pair)) {
			if (hand_defend >= 49) {
				//(7)
				if (h.call_amount == 0 && IsFlopGuilv(flop) == 0) {
					FABet(1*h.BB, 1);		//!!!加注一次
					DBONEPAIRE("7-bet 1\n");
				} else {
					FAFold();
					DBONEPAIRE("7-fold\n");
				}
			} else {
				//（8）
				if (IsFourShunzi(hand, flop) && IsMyHandShunzi(hand)) {
					// 不变
					DBONEPAIRE("8 不变\n");
				} else {
					FAFold();
					DBONEPAIRE("8-fold\n");
				}
			}		
		} else if (GetVal(flop_sec) <= GetVal(hand_pair) && GetVal(hand_pair) < GetVal(flop_max)) {
			if (hand_defend >= 49) {
				//(10)
				if (IsFourTonghua(hand, flop)) {
					// 不变
					DBONEPAIRE("10 不变\n");
				} else {
					FAFold();
					DBONEPAIRE("10-fold\n");
				}
			} else {
				//（11）
				if (IsFourShunzi(hand, flop) || IsFourTonghua(hand, flop)) {
					// 不变
					DBONEPAIRE("11 不变\n");
				} else {
					FAFold();
					DBONEPAIRE("11-fold\n");
				}
			}
		} else {
			//(13)
			if (IsFourTonghua(hand, flop)) {
				DBONEPAIRE("13-bet 1\n");
				FABet(1*h.BB, 1);		//!!!加注一次
			} else {
				FAFold();
				DBONEPAIRE("13-fold\n");
			}
		}
	}	
}

static
int commonGetMaxBet(PokerAI *ai, VarTran_t h, int type) {
	int maxBet;
	if (type == 9) {
		// 可以check时不bet，只call不bet
		maxBet = 0;
	} else if (type == 8) {
		maxBet = rand()%h.BB + h.current_bet + h.BB * 2;		
	} else {
		maxBet = h.stack;
	}
	
	return maxBet;
}

static
int commonGetCrtBet(PokerAI *ai, VarTran_t h, int type) {
	
	if (type >= 10) {
		return 0;
	} else if (type == 9) {
		return 0;
	} else if (type == 8) {
		return (rand()%h.BB + h.BB * 2)/2;
	}
	
	int ret = 0;
		
	switch (g_betno) {
	case 4:
		ret = MIN(0.3*ai->game.current_pot, MAX(h.BB*2.0, h.stack*0.1));
		break;
	case 3:
		ret = MIN(0.8*ai->game.current_pot, MAX(h.BB*2.5, h.stack*0.3));
		break;
	case 2:
		ret = MIN(1.4*ai->game.current_pot, MAX(h.BB*3.0, h.stack*0.6));
		break;
	case 1:
		ret = MIN(2.0*ai->game.current_pot, MAX(h.BB*3.5, h.stack*1));
		break;
	default:
		ret = MIN(ai->game.current_pot, h.stack);
		break;
	}
	
	return ret;
}

#endif

//=========================
//两对
static
int GetMaxPair(int *card, int n) {	
	int maxpair = 0;
	for (int i = 0; i < n; i++) {
		for (int j = i+1; j < n; j++) {
			int c1 = card[i];
			int c2 = card[j];
			int c1val  = (c1 - 1) / 4;
			int c2val  = (c2 - 1) / 4;
			if (c1val == c2val) {
				if (maxpair < c1val) {
					maxpair = c1val;
				}
			}
		}
	}
	
	return maxpair;	
}

//=================================================
//三条
static
int IsDeckcardFourShunzi(int *deckcard, int n) {

	if (n < 4) {return 0;}
	
	int *c = (int *)malloc(n*sizeof(int));
	for (int i = 0; i < n; i++) {
		c[i] = deckcard[i];
	}
	for (int i = 0; i < n; i++) {
		for (int j = i; j < n; j++) {
			if (c[i] < c[j]) {
				int t = c[i];
				c[i] = c[j];
				c[j] = t;
			}
		}
	}
	for (int i = 0; i < n-1; i++) {
		if (GetVal(c[i])-GetVal(c[i+1]) != 1) {
			return 0;
		}
	}
	
	return 1;
}

static
int IsDeckcardFourTonghua(int *deckcard, int n) {
	//int n = 5;
	if (n < 4) {return 0;}
	
	int s0=GetSuit(deckcard[0]);
	int s1=GetSuit(deckcard[1]);
	int s2=GetSuit(deckcard[2]);
	int s = s0;
	if (s0==s1) {
		s = s0;
	} else if (s0 == s2) {
		s = s0;
	} else if (s1 == s2){
		s = s1;
	} else {
		return 0;
	}

	int cnt = 0;	
	for (int i = 0; i < n-1; i++) {
		if (GetSuit(deckcard[i]) == s) {
			cnt++;
		}
	}
	
	if (cnt < 4) {
		return 0;
	}
	return 1;
}
static
int GetPair(int *deckcard, int n) {
	int maxpairval = -1;
	int maxpair    = -1;
	for (int i = 0; i < n; i++) {
		for (int j = i; j < n; j++) {
			int c1 = deckcard[i];
			int c2 = deckcard[j];
			int c1val  = (c1 - 1) / 4;
			int c2val  = (c2 - 1) / 4;
			if (c1val == c2val) {
				if (maxpairval < c1val) {
					maxpairval = c1val;
					maxpair    = c1;
				}
			}			
		}
	}
	return maxpair;	
}

static
int GetThree(int *deckcard, int n) {
	int pair = GetPair(deckcard, n);
	int cnt = 0;
	for (int i = 0; i < n; i++) {
		if (deckcard[i] == pair) {
			cnt++;
		}
	}
	if (cnt >= 3) {
		return pair;
	} else {
		return 0;
	}
}
/*
static
int FormPair(int *deckhand, int *deckcard, int n) {
	int maxpairval = -1;
	int maxpair    = -1;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < n; j++) {
			int c1 = deckhand[i];
			int c2 = deckcard[j];
			int c1val  = (c1 - 1) / 4;
			int c2val  = (c2 - 1) / 4;
			if (c1val == c2val) {
				if (maxpairval < c1val) {
					maxpairval = c1val;
					maxpair    = c1;
				}
			}			
		}
	}
	return maxpair;
}
*/	
static
void StragetFlopCommon(PokerAI *ai, VarTran_t h) {
    double winprob = ai->action.winprob;
    double expectedgain = ai->action.expectedgain;

    int randnum = rand() % 100;
	
	int cardtype = GetCardType(ai, h);
	PrintCardType(cardtype);
	//int maxbet = commonGetMaxBet(ai, h, cardtype);	    
    //int maxbet = (int)(MAX((ai->game.stack / 2.5), 10*h.BB) - (randnum / 2));    
	//maxbet = MIN(maxbet, ai->game.current_pot);	
	//maxbet -= ai->game.call_amount;
	
    //Don't bet too much on a bluff
    int bluffbet = rand()%(1+3*h.BB); //randnum * maxbet / 100 / 2;
	
	
	// 最大bet
	if (anew_round) {
		g_betno = 4;
		g_maxBet = commonGetMaxBet(ai, h, cardtype);	 
	}	
	int crtBet = commonGetCrtBet(ai, h, cardtype);		
	int maxbet = g_maxBet;
	
	g_crtBet = crtBet;
	//int crtBet = commonGetCrtBet(ai, h);
	

    if (expectedgain < 0.8 && winprob < 0.8)
    {
        if (randnum < 95)
        {
            ai->action.type = ACTION_FOLD;
        }
        else
        {
            ai->action.type = ACTION_BET;
            ai->action.bluff = true;
            ai->action.amount = bluffbet;
        }
    }
    else if ((expectedgain < 1.0 && winprob < 0.85) || winprob < 0.1)
    {
        if (randnum < 80)
        {
            ai->action.type = ACTION_FOLD;
        }
        else if (randnum < 5)
        {
            ai->action.type = ACTION_CALL;
            ai->action.bluff = true;
        }
        else
        {
            ai->action.type = ACTION_BET;
            ai->action.bluff = true;
            ai->action.amount = bluffbet;
        }
    }
    else if ((expectedgain < 1.3 && winprob < 0.9) || winprob < 0.5)
    {
        if (randnum < 60 || winprob < 0.5)
        {
            ai->action.type = ACTION_CALL;
            ai->action.bluff = false;
        }
        else
        {
            ai->action.type = ACTION_BET;
            ai->action.bluff = false;
            ai->action.amount = maxbet;
        }
    }
    else if (winprob < 0.95 || ai->game.communitysize < 4)
    {
        if (randnum < 30)
        {
            ai->action.type = ACTION_CALL;
            ai->action.bluff = false;
        }
        else
        {
            ai->action.type = ACTION_BET;
            ai->action.bluff = false;
            ai->action.amount = maxbet;
        }
    }
    else //huge chance of winning, okay to bet more than "maxbet"
    {
        //maxbet = (ai->game.stack - ai->game.call_amount) * 9 / 10;
        ai->action.type = ACTION_BET;
        ai->action.bluff = false;
        ai->action.amount = maxbet;
    }

	if (IsBet() && ai->action.bluff == false) {
		ai->action.amount = crtBet;
	}
	if (ai->action.bluff == true) {
		printf("\t\t\t\t\t\t--- !!! TRY TO BLUFF [%5d] !!! ---\n", bluffbet);
	}	
	if (ai->action.bluff) {
		if (ai->action.type == ACTION_BET) {
			if (h.current_bet < h.stack/10) {
				SetBetOrCall(ai, 20, 80, bluffbet);
			} else {
				SetCallOrFold(ai, 80, 20, 0);
			}
		}
		if (ai->action.type == ACTION_CALL) {
			if (h.current_bet < h.stack/5) {
				SetCallOrFold(ai, 20, 80, 0);
			} else {
				SetFold();
			}
		}
		if (ai->action.type == ACTION_FOLD) {
			ai->action.bluff = false;
			printf("\t\t\t\t\t\t--- !!! STOP   BLUFF [%5d] !!! ---\n", bluffbet);
		}		
	}	 
	  	
	//stack protect && raise jetton

	// 下注
		
		
	// 陷阱


	/*
static
void FABet(int amount, int times) {
	g_FlopAction.use = 1;
	g_FlopAction.type = 2;
	g_FlopAction.amount = amount;
	g_FlopAction.raise_times = times;	
}	*/

	int deckcard[5] = {-1, -1, -1, -1, -1};
//	int deckall[7]  = {-1, -1, -1, -1, -1, -1, -1};
	int deckhand[2] = {-1, -1};
	int deckalllen  = ai->game.communitysize;
	for (int i = 0; i < 2; i++)
	{
		deckhand[i]= ai->game.hand[i];
	    //deckall[i] = ai->game.hand[i];	    
	}
	for (int i = 0; i < ai->game.communitysize; i++)
	{
	    deckcard[i] = ai->game.community[i];
	    //deckall[i+2]= ai->game.community[i];
	}
	
	if (cardtype == 8 && ai->game.communitysize == 3) {
		commonCheckOnePair(ai, h);
			
		if (g_FlopAction.use && g_FlopAction.type == 2) {
			if (g_FlopAction.raise_times) {
				//SetBet(g_FlopAction.amount);
				SetBet(crtBet);
			}				
		}
		if (g_FlopAction.use && g_FlopAction.type == 0) {
			SetFold();
		}
	}
	
	int IsTwoPairOK = 0;
	if (cardtype == 7) {
		int deckcard_pair = GetMaxPair(deckcard, (deckalllen-2));
		// 两对
		if (IsMyHandPair(deckhand) == 0 && deckcard_pair == 0) {
			//我的两张单独牌，桌面无对子
			IsTwoPairOK = 1; 
		} else if (IsMyHandPair(deckhand) == 1) {
			if (deckhand[0] >= deckcard_pair) {
				IsTwoPairOK = 1;
			}
		}
		if (IsTwoPairOK == 0) {
			SetFold();
		}
	}
	int IsThreeOK = 1;
	if (cardtype == 6) {
		if (IsMyHandPair(deckhand) == 1) {
			if (IsDeckcardFourTonghua(deckcard, deckalllen-2) == 1 || IsDeckcardFourShunzi(deckcard, deckalllen-2) == 1) {
				IsThreeOK = 0;
			}
		} else {
			int hand_defend = deckhand[0];
			int deckcard_pair = GetPair(deckcard, (deckalllen-2));
			
			if (deckhand[0] == deckcard_pair || deckhand[1] == deckcard_pair) {
				hand_defend = deckhand[0];
				if (hand_defend == deckcard_pair) {
					hand_defend = deckhand[1];
				}
				if (IsDeckcardFourTonghua(deckcard, deckalllen-2) == 1 || IsDeckcardFourShunzi(deckcard, deckalllen-2) == 1) {
					IsThreeOK = 0;
				}
				if (hand_defend < 41) {
					//Q
					IsThreeOK = 0;
				}
			} else {
				deckcard_pair = GetThree(deckcard, deckalllen-2);
				hand_defend = MAX(deckhand[0], deckhand[1]);
				IsThreeOK = 0;
			}									
		}
		if (IsThreeOK == 0) {
			SetFold();
		}		
	}
	
	if (cardtype > 5) {
		if (IsDeckcardFourTonghua(deckcard, deckalllen-2) == 1 || IsDeckcardFourShunzi(deckcard, deckalllen-2) == 1) {
			SetFold();
		}
	}
	
	int IsShunziOK = 1;
	if (cardtype == 5) {
		if (IsDeckcardFourTonghua(deckcard, deckalllen-2) == 1 || IsDeckcardFourShunzi(deckcard, deckalllen-2) == 1) {
			IsShunziOK = 0;
		}
		if (IsShunziOK == 0) {
			SetFold();
		}		
	}
	
	int IsTonghuaOK = 1;
	if (cardtype == 4) {
		if (IsDeckcardFourTonghua(deckcard, deckalllen-2) == 1 || IsDeckcardFourShunzi(deckcard, deckalllen-2) == 1) {
			IsTonghuaOK = 0;
		}
		if (IsTonghuaOK == 0) {
			SetFold();
		}		
	}
/*
	if (IsCall()) {
		printf("--- maxbet %5d --- my new bet %5d crtBet %5d --- \n", g_maxBet, ai->game.current_pot + h.call_amount, crtBet);
	} else if (IsBet()) {
		printf("--- maxbet %5d --- my new bet %5d crtBet %5d --- \n", g_maxBet, ai->game.current_pot + MAX(h.call_amount + h.raise_amount, g_crtBet), crtBet);
	}
*/	
	// 检查加注值
	int maxBet = maxbet;
	
	if (IsBet()) {		
		if (maxBet < h.current_bet + h.call_amount + h.raise_amount) {
			SetBetOrCall(ai, 10, 90, crtBet);
			puts("Check Bet Error");
		} else if (maxBet - crtBet < 0 ) {
			SetBetOrCall(ai, 10, 90, 0);
			puts("Check Bet Error");
			if (h.raise_amount > 3*h.BB) {
				SetFold();
				puts("Check Bet Error");
			}
		}
		
	}
	if (IsCall()) {
		if (maxBet < h.current_bet + h.call_amount) {
			//SetCallOrFold(ai, 10, 90, crtBet);
			SetFold();
			puts("Check Call Error");
		}		
	}
/*	
	if (IsCall()) {
		printf("--- maxbet %5d --- my new bet %5d crtBet %5d --- \n", g_maxBet, ai->game.current_pot + h.call_amount, crtBet);
	} else if (IsBet()) {
		printf("--- maxbet %5d --- my new bet %5d crtBet %5d --- \n", g_maxBet, ai->game.current_pot + MAX(h.call_amount + h.raise_amount, g_crtBet), crtBet);
	}		
*/	
    //If our max bet is less than the call amount, just call instead
    if ((ai->action.type == ACTION_BET) && (maxbet < 0))
    {
        ai->action.type = ACTION_CALL;
    }
    if ((ai->action.type == ACTION_BET) && (maxbet < h.raise_amount))
    {
        ai->action.type = ACTION_CALL;
    } 
    		
	FAUpdateBet(ai);
	
}

void MakeDecisionFlop(PokerAI *ai)
{

	VarTran_t h 	= VarTran(ai);	
	int       stg   = GetFlopStraget(h.round);	

	MemcpyAction(&(ai->action_last), &(ai->action));

	//printf("===================%d %d(last)=================", ai->game.communitysize, last_communitysize);
	if (ai->game.communitysize - last_communitysize >= 1) {
		last_communitysize = ai->game.communitysize;
		anew_communitysize = 1;
		if (1) {
			//puts("--------------------+++++------------------------");
		}
	} else {
		last_communitysize    = 0;
		anew_communitysize 	  = 0;
		anew_round            = 0;
	}
	
	switch (stg) {
	case 1:
		StragetFlopCommon(ai, h);
		break;
	default:
		StragetFlopCommon(ai, h);
		break;
	}    	
	
	/*
    // All in ... !!!!!!
    if (h.wp >= 0.97) {
    	ai->action.type = ACTION_ALL_IN;
    } else if (h.wp >= 0.95 && h.allin_no_bf) {
    	// 如果前面没有人all_in 那么牌不是最牛逼也可以率先all_in
    	ai->action.type = ACTION_ALL_IN;
    }
    */
    // 钱少的时候在preflop阶段all in赢面才大，在这里all in机会很小
    // TODO
/*
	// call的值不能超过最大可下注值，没有下面的判断就可能call all_in 一盘输光
	// 如果money还有不少的时候，不必fold
	// 投入太大的时候，也不可能收手，只能搏一把，也就是说current_bet不大是call转fold的必要条件
	if (h.money < 25*40 && wpBet < 0 && h.current_bet < h.stack/2) {
		SetFold();
	}
*/  
	//OprBeforAgg(ai);
    if ((ai->action.type == ACTION_FOLD) && (ai->game.call_amount == 0)) {
    	printf("--- fold  ---> check --- \n");
        ai->action.type = ACTION_CALL;
    }
	//FAUpdateBet(ai);
	//Aggress_t agg;	
	//SetAggressStruct(&agg, 3);
	//GetAggressLevel(&agg, ai, h);
	//SetAggressAction(&agg, ai, h, IsBetterClassFlopWinprob);
    
    RecordStat(ai);
	if(IsBet()) {
		printf("--- g_betno %4d anew_round %4d --- \n", g_betno, anew_round);
		anew_round = 0;
		g_betno --;
	}

	//PrintDebugAI(ai, h, 1, 3);
}


