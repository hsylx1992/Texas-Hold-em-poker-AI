#include "action.h"
#include <string.h>

/*
 * Set the action to fold
 * action: the Action to set
 */
void ActionSetFold(Action *action)
{
    action->type = ACTION_FOLD;
    action->amount = 0;
}

/*
 * Set the action to call
 * action: the Action to set
 */
void ActionSetCall(Action *action)
{
    action->type = ACTION_CALL;
    action->amount = 0;
}

/*
 * Set the action to bet the given amount
 * action: the Action to set
 * amount: the size of the bet
 */
void ActionSetBet(Action *action, int amount)
{
    action->type = ACTION_BET;
    action->amount = amount;
}

/*
 * Get the string representation of the action
 * action: the Action to access
 */
char *ActionGetString(Action *action)
{
    switch (action->type)
    {
    case ACTION_FOLD:
        sprintf(action->string, "fold \n");
        break;

    case ACTION_CALL:
        sprintf(action->string, "call \n");
        break;

    case ACTION_BET:
        sprintf(action->string, "raise %d \n", action->amount);
        break;

	case ACTION_CHECK:
		sprintf(action->string, "check \n");
		break;
		
	case ACTION_ALL_IN:
		sprintf(action->string, "all_in \n");
		break;	
			
    default:
        //Should not be here! Action name has not been set
        sprintf(action->string, "fold \n");
        break;
    }

    return action->string;
}

char *ActionGetStringNoHuanHang(Action *action)
{
    switch (action->type)
    {
    case ACTION_FOLD:
        sprintf(action->string, "fold      ");
        break;

    case ACTION_CALL:
        sprintf(action->string, "call      ");
        break;

    case ACTION_BET:
        sprintf(action->string, "raise %.4d", action->amount);
        break;

	case ACTION_CHECK:
		sprintf(action->string, "check     ");
		break;
		
	case ACTION_ALL_IN:
		sprintf(action->string, "all_in    ");
		break;	
			
    default:
        //Should not be here! Action name has not been set
        sprintf(action->string, "error     ");
        break;
    }

    return action->string;
}

void MemcpyAction(Action *Dst, Action *Src) {
    Dst->type 	= Src->type;
    Dst->bluff 	= Src->bluff;
    Dst->amount	= Src->amount;

    Dst->winprob		= Src->winprob;
    Dst->expectedgain	= Src->expectedgain;

    memcpy(Dst->string, Src->string, sizeof(Src->string));
}
