#include<stdio.h>
#include<stdlib.h>
#include"arrays.h"
#include"eval.h"


//#include<wchar.h>
//#include<locale.h>
//#include"textcolor.h"


void srand48();
double drand48();


unsigned find_fast(unsigned u){
    unsigned a, b, r;
    u += 0xe91aaa35;
    u ^= u >> 16;
    u += u << 8;
    u ^= u >> 4;
    b  = (u >> 8) & 0x1ff;
    a  = (u + (u << 2)) >> 19;
    r  = a ^ hash_adjust[b];
    return r;
}
void init_deck( int *deck ){
	//setlocale(LC_CTYPE,"en_US.utf8");
    int i, j, n = 0, suit = 0x8000;
    for ( i = 0; i < 4; i++, suit >>= 1 )
        for ( j = 0; j < 13; j++, n++ )
            deck[n] = primes[j] | (j << 8) | suit | (1 << (16+j));
}
int find_card( int rank, int suit, int *deck ){
	int i, c;
	for ( i = 0; i < 52; i++ ){
		c = deck[i];
		if ( (c & suit)  &&  (RANK(c) == rank) )
			return( i );
	}
	return( -1 );
}
void shuffle_deck( int *deck ){
    int i, n, temp[52];
    for ( i = 0; i < 52; i++ )
        temp[i] = deck[i];
    for ( i = 0; i < 52; i++ ){
        do {
            n = (int)(51.9999999 * drand48());
        } while ( temp[n] == 0 );
        deck[i] = temp[n];
        temp[n] = 0;
    }
}
void print_hand( int *hand, int n ){
    int i, r;
    //static wchar_t rank[13]=L"23456789TJQKA";
    //static wchar_t suit;
    char rank[13] = "23456789TJQKA";
    char suit;
    for(i=0;i<n;i++){
        r = (*hand >> 8) & 0xF;
        if ( *hand & CLUB)
            //suit=L'♣';
            suit = '+';
        else if ( *hand & DIAMOND ){
        	//textcolor(RESET,RED,WHITE);
            //suit=L'♦';
            suit='-';
        }
        else if ( *hand & HEART ){
        	//textcolor(RESET,RED,WHITE);
            //suit=L'♥';
            suit='*';
        }
        else
        	//suit=L'♠';
            suit='/';
        hand++;
        if(r==8)
        	//wprintf(L"10%lc ",suit);
        	printf("10%c ",suit);
		else
			printf("%c%c ",rank[r],suit);
	        //wprintf(L"%lc%lc ",rank[r],suit);
        //textcolor(RESET,BLACK,WHITE);
    }
    printf("%c", '\n');
}
int hand_rank( short val ){
    if (val > 6185) return(HIGH_CARD);
    if (val > 3325) return(ONE_PAIR);
    if (val > 2467) return(TWO_PAIR);
    if (val > 1609) return(THREE_OF_A_KIND);
    if (val > 1599) return(STRAIGHT);
    if (val > 322)  return(FLUSH);
    if (val > 166)  return(FULL_HOUSE);
    if (val > 10)   return(FOUR_OF_A_KIND);
    return(STRAIGHT_FLUSH);
}
int eval_5cards(int c1, int c2, int c3, int c4, int c5){
    int q = (c1|c2|c3|c4|c5) >> 16;
    short s;
    if(c1&c2&c3&c4&c5&0xF000)return flushes[q];
    if((s=unique5[q]))return s;	//!!!!!!这里确定是赋值吗？？？？
    return hash_values[find_fast((c1&0xff)*(c2&0xff)*(c3&0xff)*(c4&0xff)*(c5&0xff))];
}
short eval_5hand( int *hand ){
    int c1, c2, c3, c4, c5;
    c1 = *hand++;
    c2 = *hand++;
    c3 = *hand++;
    c4 = *hand++;
    c5 = *hand;
    return( eval_5cards(c1,c2,c3,c4,c5) );
}
short eval_6hand( int *hand ) {
	short rank[6], minrank=9999, i;
	
	rank[0]=eval_5cards(        hand[1],hand[2],hand[3],hand[4],hand[5]);
	rank[1]=eval_5cards(hand[0],        hand[2],hand[3],hand[4],hand[5]);
	rank[2]=eval_5cards(hand[0],hand[1],        hand[3],hand[4],hand[5]);	
	rank[3]=eval_5cards(hand[0],hand[1],hand[2],        hand[4],hand[5]);
	rank[4]=eval_5cards(hand[0],hand[1],hand[2],hand[3],        hand[5]);
	rank[5]=eval_5cards(hand[0],hand[1],hand[2],hand[3],hand[4]        );	
	
	//printf("%d %d %d %d %d %d\n", rank[0], rank[1], rank[2], rank[3], rank[4], rank[5]);
	
	for (i=0;i<6;i++) {
		if (minrank > rank[i]) {
			minrank = rank[i];
		}		
	}
	
	return minrank;
}
short eval_6cards_hsy(int c1, int c2, int c3, int c4, int c5, int c6) {
	int *hand = (int *)malloc(6*sizeof(int));
	hand[0]	= c1;
	hand[1]	= c2;
	hand[2]	= c3;
	hand[3]	= c4;
	hand[4]	= c5;
	hand[5]	= c6;
	return eval_6hand(hand);
}
short eval_7hand( int *hand ){
    int i, j, q, best = 9999, subhand[5];
	for ( i = 0; i < 21; i++ ){
		for ( j = 0; j < 5; j++ )
			subhand[j] = hand[ perm7[i][j] ];
		q = eval_5hand( subhand );
		if ( q < best )
			best = q;
	}
	return( best );
}
short eval_7hand_hsy( int *hand ){
	short rank[7], minrank=9999, i;
	
	rank[0]=eval_6cards_hsy(        hand[1],hand[2],hand[3],hand[4],hand[5],hand[6]);
	rank[1]=eval_6cards_hsy(hand[0],        hand[2],hand[3],hand[4],hand[5],hand[6]);
	rank[2]=eval_6cards_hsy(hand[0],hand[1],        hand[3],hand[4],hand[5],hand[6]);
	rank[3]=eval_6cards_hsy(hand[0],hand[1],hand[2],        hand[4],hand[5],hand[6]);
	rank[4]=eval_6cards_hsy(hand[0],hand[1],hand[2],hand[3],        hand[5],hand[6]);
	rank[5]=eval_6cards_hsy(hand[0],hand[1],hand[2],hand[3],hand[4],        hand[6]);
	rank[6]=eval_6cards_hsy(hand[0],hand[1],hand[2],hand[3],hand[4],hand[5]        );
	
	//printf("%d %d %d %d %d %d %d\n", rank[0], rank[1], rank[2], rank[3], rank[4], rank[5], rank[6]);
	
	for (i=0;i<7;i++) {
		if (minrank > rank[i]) {
			minrank = rank[i];
		}		
	}
	
	return minrank;
}
short eval_hand( int *hand, int n ){
	if (n == 5) return eval_5hand(hand);
	if (n == 6) return eval_6hand(hand);
	if (n == 7) return eval_7hand(hand);

	return 9999;
}

static void
MyHandToEvalHand_Int(int *hand, int n) {
	int rank, suit, i;
	int deck[52];
	
	init_deck( deck );
	
	for (i = 0; i < n; i++) {
		//hand[i] = i;
		
		rank = (hand[i]-1)/4;
		suit = (hand[i]-1)%4;
			
		if (suit == 0) {
			suit = 3;
		} else {
			suit--;
		}
		
		int eval_card_unfm = suit*13 + rank;

		hand[i] = deck[eval_card_unfm];
	}
}

short eval_hand_hsy( int *hand, int n ){
	MyHandToEvalHand_Int(hand, n);
	return eval_hand(hand, n);
}
