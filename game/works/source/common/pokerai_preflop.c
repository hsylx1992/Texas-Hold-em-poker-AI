#include "pokerai.h"
//#include "gamestate.h"
#include "pokerai_aggress.h"

//#include "pokerai_preflop.h"


int last_round_id = 0;
int anew_round = 0;

// 配置
//static float grpclasscommon[] = {-1, 1, 2.4, 3, 4};
// 差 中 好
// 19 16 14
//static int pvclasscommon[] = {};
//static int pvTh = 14;
static int g_pvFRONT	= 19;
static int g_pvMIDDLE  	= 15;
static int g_pvBACK    	= 13;
static int g_pvMIN	 	= 11;	// first small blind

static int g_maxBet;
//==========================================================================================================================
    
static
int GetCurrentStraget(int round) {
	
	int stg = 1;
	int mod = (int)(round / 60);
	if (mod%2 == 0) {
		stg = 1;
	} else {
		stg = 1;
	}
	return stg;
}

//==========================================================================================================================
#if 0
static
void set_grpclasscommon(int a1, int a2, int a3, int a4) {
}
#endif


PosType
NewTranPos(PokerAI *ai, VarTran_t h) {
	
	// 在外面处理SB BB	
	
	if (h.noPlaying >= 7){			
		if (h.pos_type == POSTYPE_SB) {
			return POSTYPE_FRONT;
		} else if (h.pos_type == POSTYPE_BB) {
			return POSTYPE_FRONT;
		} else {
			return h.pos_type;
		}		
	} else if (h.noPlaying <= 3) {
		//TODO:我们假设这时候只有两三个玩家，因而位置论没有作用
		return POSTYPE_BACK;
	} else {
		return h.pos_type;
	}
	//printf("[%d]%d %d %d\n", ret, pos, blind_no, player_no);
	//return pos_type;
}

static
int commonGetPh(PokerAI *ai, VarTran_t h) {
	int ret = 10;
	
	ai->game.pos_type = NewTranPos(ai, h);
	
	if (h.pos_type == POSTYPE_BACK) {
		ret = g_pvBACK;
	} else if (h.pos_type == POSTYPE_MIDDLE) {
		ret = g_pvMIDDLE;
	} else {
		//h.pos_type == POSTYPE_BACK)
		ret = g_pvFRONT;
	}
		
	//SB BB的位置好坏需要特殊处理	
	if (h.pos_type == POSTYPE_SB || h.pos_type == POSTYPE_BB) {
		if (anew_round == 1) {
			ai->game.pos_type = POSTYPE_BACK;
			ret = g_pvMIN;		
		} else {
			ai->game.pos_type = POSTYPE_FRONT;
			ret = g_pvFRONT;			
		}
	}	
	
	return ret;
}

static
int commonGetMaxBet(PokerAI *ai, VarTran_t h) {
	int maxBet = 0;
	
	if (h.pv >= g_pvFRONT) {
		maxBet = h.BB*8+rand()%(h.BB);
	} else if (h.pv >= g_pvMIDDLE) {
		maxBet = h.BB*6+rand()%(h.BB);
	} else if (h.pv >= g_pvBACK) {
		maxBet = h.BB*5+rand()%(h.BB);
	} else if (h.pv >= g_pvMIN) {
		maxBet = h.BB*4+rand()%(h.BB)+10;
	} else {
		maxBet = 0;
	}
	
	// 超好牌、快结束，死跟到底
	if (h.pv >= 30 && h.round >= 500) {
		// AA KK		
		maxBet = h.stack;	//!!!!!
	} else if (h.pv >= 24 && h.round >= 450) {
		// QQ JJ AKs
		maxBet = h.stack;
	}
		
	return maxBet;
}

static
int commonGetCrtBet(PokerAI *ai, VarTran_t h) {

	int level = 0;

	if (h.pv >= 30) {
		// AA KK
		level = 4;
	} else if (h.pv >= 24) {
		// QQ JJ AKs
		level = 4;
	} else if (h.pv >= g_pvFRONT) {
		level = 3;
	} else if (h.pv >= g_pvMIDDLE){
		level = 2;
	} else if (h.pv >= g_pvBACK) {
		level = 1;
	}

	int intelevel = h.pos_type - level;
	if (intelevel > 3) {intelevel = 3;}
	if (intelevel < 0 ) {intelevel = 0;}	
	
	int crtBet = h.BB + (intelevel+1)*(rand()%5);
	
	// 尝试吃入盲注
	if (h.pv < g_pvMIDDLE) {
		crtBet = 2*h.BB + 2 + rand()%5;
	}
	return crtBet;
}

static
int commonCanEntry(PokerAI *ai, VarTran_t h)
{
	if (h.pv >= commonGetPh(ai, h)) {
		return 1;
	} else {
		return 0;
	}
}
static
int StragetPreflopCommon(PokerAI *ai, VarTran_t h, int (*CanEntry)(PokerAI *ai, VarTran_t h)) {
	// 设置阈值
	//
	
	// 是否玩该手牌
	if (!CanEntry(ai, h)) {
		SetFold();
		return -1;
	}
	
	// 最大bet
	if (anew_round) {
		g_maxBet = commonGetMaxBet(ai, h);
		//g_crtBet = commonGetCrtBet(ai, h);
	}
	int maxBet = g_maxBet;
	int crtBet = commonGetCrtBet(ai, h);
		
	// 严重错误警告
	if (crtBet < 0) {
		printf("---!!!crtBet ERROR!!!---\n");
	}	
	if (h.pv <= g_pvMIN) {
		printf("---!!!pv ERROR!!!---should:%2d < %2d ---\n", h.pv, g_pvMIN);
	}		

	// 投注
	if (maxBet >= h.current_bet + h.call_amount) {
		if (crtBet > 0) {
			SetBetOrCall(ai, 90, 10, crtBet);			
		} else if (crtBet == 0) {
			SetCall();
		} else {
			SetFold();
			//SetCallOrFold(ai, 10, 90, 0);
		}
	} else {
		SetFold();
		//SetCallOrFold(ai, 10, 90, 0);
	}
	
	// 检查加注值
	if (IsBet()) {		
		if (maxBet < h.current_bet + h.call_amount + h.raise_amount) {
			SetBetOrCall(ai, 10, 90, crtBet);
		}
	} else if (IsCall()) {
		if (maxBet < h.current_bet + h.call_amount) {
			//SetCallOrFold(ai, 10, 90, crtBet);
			SetFold();
		}		
	}	
		
	// 可以全跟的状况
	if (IsFold() && maxBet >= h.stack) {
		SetBetOrCall(ai, 90, 10, crtBet);
	}

	// 是否可以连续加注 是否可以再跟住
	int tmc = 0;//>=h.raise_no_c;
	int rra = 0;//re-raise abalable
		
	// 更新状态
	if (h.pv >= 24) {
		tmc = 0;
		rra = 1;
	}
	
	if (h.pv <= g_pvBACK) {
		tmc = 0;
		rra = 0;
	}
		
	// tmc - h.raise_no_c
	// 当前为raise 前面也曾raise，那么当前的raise会使得raise_no_c加一
	if ( IsBet() && ai->game.raise_no > 0 && h.raise_no_c+1 > tmc ) {
		int bet = 0;
		if (h.pv >= 24) {
			puts("1");
			SetBetOrCall(ai, 20,  80, bet);
		} else if (h.pv >= g_pvFRONT) {
			puts("2");
			SetCall();
		} else if (h.pv >= g_pvMIDDLE) {
			puts("3");
			SetCall();
		} else if (h.pv >= g_pvBACK) {
			puts("4");
			SetCall();			
		} else {
			puts("!!!");
			SetFold();
		}		
	}
	// rra - h.raise_no_bf
	// 当前为raise 前面有其他玩家raise，那么当前的raise是re-raise
	if ( IsBet() && h.raise_no_bf > rra ) {
		if (h.raise_no_c || (h.raise_no_c == 0 && h.raise_no == 1)) {
			if (rra >= 1) {
				SetBetOrCall(ai, 10,  90, crtBet);
			} else {
				SetFold();
			}
		}
	}
	// 前面有很多玩家跟注
	if (IsBet() && h.call_no_bf >= 3) {
		if (h.pv >= 24) {
			// AA KK QQ JJ AKs
			//bet --> bet
		} else {			
			SetBetOrCall(ai, 10,  90, crtBet);			
		}
	}
	
	// end protect
	
	return 0;
}

#if 0
static
int IsBetterClassPreflopPwr(int pwr, int class) {	
//	h.pwr >= 10-aggPWCLASSFOUR	//!!!
//	int aggPWCLASS[4]   = {1, 3, 5, 7};
	int aggPWCLASS[4]   = {9, 7, 5, 3};
	if (pwr >= aggPWCLASS[class]) {
		return 1;
	} else {
		return 0;
	}
}
#endif

//==========================================================================================================================

//void SetAggressAction(Aggress_t *agg, PokerAI *ai, VarTran_t h, int (*IsBetter)(int pwr, int class));
//void GetAggressLevel(Aggress_t *agg, PokerAI *ai, VarTran_t h);

void MakeDecisionPreflop(PokerAI *ai) {

	VarTran_t h 	= VarTran(ai);
	int       stg   = GetCurrentStraget(h.round);	//1

	MemcpyAction(&(ai->action_last), &(ai->action));

	if (h.round - last_round_id >= 2) {		
		last_round_id = h.round-1;
		anew_round    = 1;
		if (1) {
			//puts("---------------------------------------------------------------------");
		}
	} else {
		anew_round 	  = 0;
	}
	
	switch (stg) {
	case 1:		
		StragetPreflopCommon(ai, h, commonCanEntry);
		break;
	case 2:
		StragetPreflopCommon(ai, h, commonCanEntry);
		break;
	default:
		StragetPreflopCommon(ai, h, commonCanEntry);
		break;
	}
		
	//OprBeforAgg(ai);
    if ((ai->action.type == ACTION_FOLD) && (ai->game.call_amount == 0)) {
    	printf("--- fold  ---> check --- \n");
        ai->action.type = ACTION_CALL;
    }	
	
	//Aggress_t agg;	
	//SetAggressStruct(&agg, 0);
	//GetAggressLevel(&agg, ai, h);
	//SetAggressAction(&agg, ai, h, IsBetterClassPreflopPwr);
		
	RecordStat(ai);	// always last
	
	//PrintDebugAI(ai, h, 1, 0);	//(h.pv >= g_pvMIN)
}


