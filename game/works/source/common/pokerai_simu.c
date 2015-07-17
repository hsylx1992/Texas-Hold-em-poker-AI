#include "pokerai.h"
#include "pokerai_aggress.h"

//Naive way to get thread ID
#define THREAD_ID ((unsigned int)pthread_self() % 100)


#if (SIMU_ALG_TYPE == 1)
	int hsySimuAlgType = 1;
#endif

#if (SIMU_ALG_TYPE == 2)
	int hsySimuAlgType = 2;
#endif

#if (SIMU_ALG_TYPE == 3)
	int hsySimuAlgType = 3;
#endif

static
void SpawnMonteCarloThreads(PokerAI *ai);

static
void *SimulateGames(void *_ai);

static
int SimulateSingleGame(PokerAI *ai, int seed_index);

static
int draw(int *deck, int *psize, int rand_num);

static
int BestOpponentHand(int **opponents, int numopponents, int numcards);

static
int GetNextFreeSeedIndex(PokerAI *ai);

static
void ReleaseSeedIndex(PokerAI *ai, int index);

static
double PreflopWinProbability(int *hand);

static
int draw(int *deck, int *psize, int rand_num)
{
    int index = rand_num % *psize;
    int value = deck[index];
    deck[index] = deck[*psize - 1];
    *psize -= 1;

    return value;
}

static
void try_draw_hand(PokerAI *ai, int seed_index, int *deck, int *psize, int *hand, int *index)
{
	int rand_num;
	int value;
	
	hand[0] = -1;
	hand[1] = -1;
	// TODO NUM_HAND == 2
    ai->seeds[seed_index] = rand_r((unsigned int *)&ai->seeds[seed_index]);
    rand_num  = ai->seeds[seed_index];
    index[0]  = rand_num % *psize;
    value 	  = deck[index[0]];
    hand[0]   = value;
    
    do {
    ai->seeds[seed_index] = rand_r((unsigned int *)&ai->seeds[seed_index]);
    rand_num  = ai->seeds[seed_index];
    index[1]  = rand_num % *psize;
    value 	  = deck[index[1]];
    hand[1]   = value;    
    } while (hand[0] == hand[1]);
    
}

static
void con_draw_hand(PokerAI *ai, int seed_index, int *deck, int *psize, int *hand, int *index) {
    deck[index[0]] = deck[*psize - 1];
    *psize -= 1;
    deck[index[1]] = deck[*psize - 1];
    *psize -= 1;    
}

static
void my_draw(PokerAI *ai, int seed_index, int *deck, int *psize, int *hand) 
{
	int index[2] = {-1, -1};
	do {
		try_draw_hand(ai, seed_index, deck, psize, hand, index);
	} while	(GetPreflopValue(hand) < 10);
	con_draw_hand(ai, seed_index, deck, psize, hand, index);	
}

static
int BestOpponentHand(int **opponents, int numopponents, int numcards)
{
    int best = 9999;
    int score;
    for (int i = 0; i < numopponents; i++)
    {
        score = eval_hand_hsy(opponents[i], numcards);
        if (best > score)
        {
            best = score;
        }
    }

    return best;
}

static
int GetNextFreeSeedIndex(PokerAI *ai)
{
    int index = -1;

    pthread_mutex_lock(&ai->seed_mutex);

    for (int i = 0; i < ai->num_threads; i++)
    {
        if (ai->seed_avail[i])
        {
            ai->seed_avail[i] = false;
            index = i;
            break;
        }
    }

    pthread_mutex_unlock(&ai->seed_mutex);
    return index;
}


static
void ReleaseSeedIndex(PokerAI *ai, int index)
{
    pthread_mutex_lock(&ai->seed_mutex);
    ai->seed_avail[index] = true;
    pthread_mutex_unlock(&ai->seed_mutex);
}

static
int SimulateSingleGame(PokerAI *ai, int seed_index)
{
    GameState *game = &ai->game;
    int me[NUM_HAND + NUM_COMMUNITY];
    int *opponents[MAX_OPPONENTS];
    int community[NUM_COMMUNITY];
    int myscore;
    int bestopponent;
    int rand_num;
	int hsySimPlayers = game->num_playing;	//crt_player_no num_playing TODO

if (hsySimuAlgType == 2) 
{
	hsySimPlayers = game->crt_player_no - 1;
}

    for (int i = 0; i < hsySimPlayers; i++)
    {
        opponents[i] = malloc(sizeof(*opponents[i]) * (NUM_HAND + NUM_COMMUNITY));
    }

    //Create a deck as a randomized queue data structure
    int deck[NUM_DECK] = {0};
    int decksize = 0;

    //Cards are 1 indexed
    for (int i = 1; i < NUM_DECK; i++)
    {
        if (game->deck[i])
        {
            deck[decksize] = i;
            decksize++;
        }
    }

    //Add the known community cards to the simulation community list
    for (int i = 0; i < game->communitysize; i++)
    {
        community[i] = game->community[i];
    }

    //Distribute the rest of the community cards
    for (int i = game->communitysize; i < NUM_COMMUNITY; i++)
    {
        ai->seeds[seed_index] = rand_r((unsigned int *)&ai->seeds[seed_index]);
        rand_num = ai->seeds[seed_index];
        community[i] = draw(deck, &decksize, rand_num);
    }

    //Give each opponent their cards
    for (int opp = 0; opp < hsySimPlayers; opp++)
    {    
        //Personal cards
if (hsySimuAlgType == 3) 
{
        my_draw(ai, seed_index, deck, &decksize, opponents[opp]);
}
else
{
        for (int i = 0; i < NUM_HAND; i++)
        {
            ai->seeds[seed_index] = rand_r((unsigned int *)&ai->seeds[seed_index]);
            rand_num = ai->seeds[seed_index];
            opponents[opp][i] = draw(deck, &decksize, rand_num);
        }
}       
        
        //Community cards
        for (int i = NUM_HAND; i < NUM_COMMUNITY + NUM_HAND; i++)
        {
            opponents[opp][i] = community[i - NUM_HAND];
        }
    }

    //Put my cards into a new array
    for (int i = 0; i < NUM_HAND; i++)
    {
        me[i] = game->hand[i];
    }
    for (int i = NUM_HAND; i < NUM_HAND + NUM_COMMUNITY; i++)
    {
        me[i] = community[i - NUM_HAND];
    }

    //See who won
    myscore = eval_hand_hsy(me, NUM_HAND + NUM_COMMUNITY);
    bestopponent = BestOpponentHand(opponents, hsySimPlayers, NUM_HAND + NUM_COMMUNITY);

    //Free allocated memory
    for (int i = 0; i < hsySimPlayers; i++)
    {
        free(opponents[i]);
    }

    //Count ties as a win
    return (myscore <= bestopponent);
}


static
void *SimulateGames(void *_ai)
{
    PokerAI *ai = (PokerAI *)_ai;
    Timer timer;
    int seed_index = GetNextFreeSeedIndex(ai);

    if (ai->loglevel >= LOGLEVEL_DEBUG)
    {
        fprintf(ai->logfile, "[Thread %u] starting (obtained seed index %d)\n", THREAD_ID, seed_index);
    }

    int simulated = 0;
    int won = 0;

    StartTimer(&timer);
    //Only check the timer after every 5 simulations
    while (1)
    {
        if (simulated % 1000 == 0 && GetElapsedTime(&timer) > ai->timeout)
        {
            break;
        }

        won += SimulateSingleGame(ai, seed_index);
        simulated++;
    }

    if (ai->loglevel >= LOGLEVEL_DEBUG)
    {
        fprintf(ai->logfile, "[Thread %u] done\t(simulated %d games)\n", THREAD_ID, simulated);
    }

    //Release our random seed
    ReleaseSeedIndex(ai, seed_index);

    //Lock the AI mutex and update the totals
    pthread_mutex_lock(&ai->mutex);
    ai->games_won += won;
    ai->games_simulated += simulated;
    pthread_mutex_unlock(&ai->mutex);

    return NULL;
}

static
void SpawnMonteCarloThreads(PokerAI *ai)
{
    if (ai->loglevel >= LOGLEVEL_DEBUG)
    {
        fprintf(ai->logfile, "Spawning Monte Carlo threads.\n");
    }

    //Spawn threads to perform Monte Carlo simulations
    for (int i = 0; i < ai->num_threads; i++)
    {
        pthread_create(&ai->threads[i], NULL, SimulateGames, ai);
    }

    //Wait until each thread has finished simulating games
    for (int i = 0; i < ai->num_threads; i++)
    {
        pthread_join(ai->threads[i], NULL);
    }

    if (ai->loglevel >= LOGLEVEL_DEBUG)
    {
        fprintf(ai->logfile, "All Monte Carlo threads finished.\n");
    }
}

static
double PreflopWinProbability(int *hand)
{
    /*
     * Quick guide:
     * val:     0  1  2  3  4  5  6  7  8  9  10 11 12
     * card:    2  3  4  5  6  7  8  9  T  J  Q  K  A
     * score([AH,AD]) = 30
     * score([AH,KD]) = 23
     * score([TD,8D]) = 18
     * score([AH,TD]) = 16
     * score([KH,TD]) = 16
     * score([JD,7D]) = 14
     * score([4D,5D]) = 9
     * score([2H,2D]) = 6
     * score([2H,7H]) = 5
     * score([2H,7D]) = 1
     */

    int score  = 0;
    int c1val  = (hand[0] - 1) / 4;
    int c2val  = (hand[1] - 1) / 4;
    int c1suit = (hand[0] - 1) % 4;
    int c2suit = (hand[1] - 1) % 4;
    int diff = abs(c1val - c2val);

    //Be warned: many magic numbers exist in this function
    //They have been chosen based on logic fine-tuning

    score = c1val + c2val;

    //Give bonuses to valued pairs and suited pairs
    if (c1val == c2val)
    {
        score += 6;
    }
    else if (c1suit == c2suit)
    {
        score += 4;
    }

    //Lower the score of cards with a large value difference
    if (diff > 4)
    {
        score -= 4;
    }
    else if (diff > 2)
    {
        score -= diff;
    }

    return score / 30.0;
}

double GetWinProbability(PokerAI *ai)
{
    double winprob;
    ai->games_won = 0;
    ai->games_simulated = 0;

    //Use preflop statistics if there aren't any community cards yet
    if (ai->game.communitysize == 0)
    {
        if (ai->loglevel >= LOGLEVEL_DEBUG)
        {
            fprintf(ai->logfile, "Performing simple preflop computation.--SHOULD NOT BE HERE!!!\n");
        }

        winprob = PreflopWinProbability(ai->game.hand);
        //winprob = GetPreflopValue(ai->game.hand)/60;
    }
    //Otherwise, start spawning Monte Carlo threads
    else
    {
        if (ai->loglevel >= LOGLEVEL_DEBUG)
        {
            fprintf(ai->logfile, "Performing Monte Carlo simulations.\n");
        }

        SpawnMonteCarloThreads(ai);
        winprob = ((double) ai->games_won) / ai->games_simulated;

        if (ai->loglevel >= LOGLEVEL_INFO)
        {
            //Human-readable output
            if (ai->games_simulated > 1000000)
            {
                fprintf(ai->logfile, "Simulated %.3fM games.\n", (double)ai->games_simulated / 1000000);
            }
            else if (ai->games_simulated > 1000)
            {
                fprintf(ai->logfile, "Simulated %dk games.\n", ai->games_simulated / 1000);
            }
            else
            {
                fprintf(ai->logfile, "Simulated %d games.\n", ai->games_simulated);
            }
        }
    }

    return winprob;
}

