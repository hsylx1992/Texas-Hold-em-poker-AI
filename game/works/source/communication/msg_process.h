
#ifndef _MSG_PROCESS_H_
#define _MSG_PROCESS_H_


#include "msg_process.h"


/*
最长单词 	12 "small blind"
最长行		26 "small blind:1111 2000 8000"
最长消息		XX inquire+blind
 */
#define WORD_SIZE 50
#define ROW_SIZE 500
#define MSG_SIZE 4096


typedef struct stMsgRow__t {
	int  	m_nType;			//1\2\3
	char 	m_szWordBegin[WORD_SIZE];		//冒号前的内容，不一定有
	int		m_nWordNo;							//冒号后面的单词数量
	char*	m_szWordRest[10];		//冒号后面的单词数量
	
} stMsgRow_t;

typedef struct stMsg__t {
	int 	m_tRecvTime;	//接收时间
	int		m_nSeq;			//序号
	char	m_szType[WORD_SIZE];
	int		m_nType;			//5-16，意义同szType
	int		m_nRowNo;			//有多少航
	stMsgRow_t	*m_pstMsgRow[10];
	
} stMsg_t;

typedef struct stMsgs__t {
	int		m_nMsgNo;
	stMsg_t*m_stMsg[10];
	
} stMsgs_t;

char*PreShowdown(char *p);
char*PreprocessMsgs(char *p);
//int  PreprocessMsgs(char ou[MSG_SIZE], const char p[MSG_SIZE]);
int  FmMsgs(char sz[MSG_SIZE], stMsgs_t *pstMsgs);

int  Msg_FmMsgs(char sz[MSG_SIZE], stMsgs_t *pstMsgs);

void Msg_SaveSzMsgs(char sz[MSG_SIZE], FILE* fd);
void Msg_PrintStMsgs(stMsgs_t stMsg, FILE * fd);

#endif
