#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg_process.h"

#define CM '*'	// 分割msg的标记
#define CT '/'
#define CR '+'
#define CB ':'
#define CW ' '


#define DEBUG_STROUT 0


#if DEBUG_STROUT
#else
#define printf(...)	
#define puts(...) 
#endif


char*
PreShowdown(char *p)
{
	return p;
}

char*
PreprocessMsgs(char *p)
{

	p = PreShowdown(p);
//	int  ism = 0;	// 是否需要分割消息	
	char *pLstXieGang = NULL;
	char *pCrtXieGang = NULL;
	char *pLstHuanHang = NULL;
	char *ou = (char*)malloc(MSG_SIZE*sizeof(int));
	char *pou = ou;

	while (*p) {
		if (*p == '/') {

			pCrtXieGang = p;
			if (pLstXieGang && pLstHuanHang) {
				if (*(pLstXieGang-1)=='\n' && *(pCrtXieGang+1)==' ') {
					// 当前的/是type开始	// 前一个/是type结束
					// 需要分割
					*pLstHuanHang = '*';
					//puts("......splited");
				}
			}			
			pLstXieGang = pCrtXieGang;

			*pou++ = *p++;
		} else if (*p == '\n') {
			if (*(p-2) == '/') {
				//*(pou-2) = '+';
				//pLstHuanHang = pou-2;
				pou--;
			} else {
				*(pou-1) = '+';
				pLstHuanHang = pou-1;
			}
			p += 1;
		} else if (*p == ':') {
			*pou++ = ':';
			p += 2;
		} else {		
			*pou++ = *p;
			p += 1;
		}
	}
	*(pou-1) = '\0';	//去掉最后一个换行

	return ou;
}

char*
FmOneRowPreprocess(char *prow, int *ptype)
{
	char *ou = (char*)malloc(ROW_SIZE);
	char *pou= ou;
	
	*ptype = 1;

	while (*prow) {
		if (*prow == ':') {
			*ptype = 2;
			*pou++ = *prow++;
		} else if (*prow == '|') {
			*ptype = 3;
			pou--;
			prow++;
		} else {
			*pou++ = *prow++;
		}
	}
	*pou = '\0';

	return ou;
}

int
FmOneRow(char *prow, stMsgRow_t *pstMsgRow)
//FmOneRow(char *prow)
{
	char *pwordbegin, *pwordrests, *pword;
	char *p;
	int  type;

	prow = FmOneRowPreprocess(prow, &type);
	
	pstMsgRow->m_nType = type;						//!st
	pstMsgRow->m_nWordNo = 0;

	pwordbegin=strtok(prow, ":");
	pwordrests=strtok(NULL, ":");
	if (!pwordrests) {
		// 没有类型，特殊处理
		char szwordbegin[] = "normal";
		pwordrests=pwordbegin;
		pwordbegin=szwordbegin;			
	}
	printf("------[begi]%s\n", pwordbegin);
	strcpy(pstMsgRow->m_szWordBegin, pwordbegin);	//!!!st
	pword = strtok_r(pwordrests, " ", &p);
	while (pword) {
		printf("------[word]%s\n", pword);
		pstMsgRow->m_szWordRest[pstMsgRow->m_nWordNo] = (char *)malloc(sizeof(char[WORD_SIZE]));
		strcpy(pstMsgRow->m_szWordRest[pstMsgRow->m_nWordNo++], pword);	//!!!st

		pword = strtok_r(NULL, " ", &p);		
	}
	
	return 0;
}

int
FmOneinfo(char *pinfo, stMsg_t *pstOneMsg)
{
	char *prow;
	char *p;

	//stMsgRow_t stMsgRow;	//pstOneMsg->m_pstMsgRow[i]
	pstOneMsg->m_nRowNo = 0;

	//分割info
	prow = strtok_r(pinfo, "+", &p);
	
	while (prow) {
		printf("----[row]%s\n", prow);
		pstOneMsg->m_pstMsgRow[pstOneMsg->m_nRowNo] = (stMsgRow_t *)malloc(sizeof(stMsgRow_t[ROW_SIZE]));
		FmOneRow(prow, pstOneMsg->m_pstMsgRow[pstOneMsg->m_nRowNo++]);

		prow = strtok_r(NULL, "+", &p);
	}	

	return 0;
}

void
TurnMsgType(stMsg_t *pstOneMsg)
{
	char type[WORD_SIZE];
	int  ret = -1;

	strcpy(type, pstOneMsg->m_szType);
	if (strcmp(type, "seat") == 0) {
		ret = 6;
	} else if (strcmp(type, "blind") == 0) {
		ret = 8;
	} else if (strcmp(type, "hold") == 0) {
		ret = 9;
	} else if (strcmp(type, "inquire") == 0) {
		ret = 10;
	} else if (strcmp(type, "flop") == 0) {
		ret = 12;
	} else if (strcmp(type, "turn") == 0) {
		ret = 13;
	} else if (strcmp(type, "river") == 0) {
		ret = 14;
	} else if (strcmp(type, "showdown") == 0) {
		ret = 15;
	} else if (strcmp(type, "pot-win") == 0) {
		ret = 16;
	} else {
		ret = -1;
	}
	
	pstOneMsg->m_nType = ret;
}

int
FmOneMsg(char *pmsg, stMsg_t *pstOneMsg)
{	
//	char *p;

	//分割type\info\type
	char *ptypea = strtok(pmsg, "/");
	char *pinfo  = strtok(NULL, "/");
	char *ptypeb = strtok(NULL, "/");
	
	if (0 != strcmp(ptypea, ptypeb)){
		;
	}
	//stMsg_t stOneMsg;
	pstOneMsg->m_tRecvTime = 0;
	pstOneMsg->m_nSeq      = 0;
	strcpy(pstOneMsg->m_szType, ptypea);
	TurnMsgType(pstOneMsg);

	printf("--[typea]%s[info]%s[typeb]%s\n", ptypea, pinfo, ptypeb);
	
	FmOneinfo(pinfo, pstOneMsg);

	return 0;
}

/*
 *sz 经过预处理
 */
int
FmMsgs(char sz[MSG_SIZE], stMsgs_t *pstMsgs)
{	
	char *pmsg, *p;
	
	pstMsgs->m_nMsgNo = 0;

	//分割出每一个msg
	pmsg = strtok_r(sz, "*", &p);
	
	while (pmsg) {
		printf("|---------------OneMsg---------------|\n[msg]%s\n", pmsg);
		pstMsgs->m_stMsg[pstMsgs->m_nMsgNo] = (stMsg_t *)malloc(sizeof(stMsg_t[MSG_SIZE]));
		FmOneMsg(pmsg, pstMsgs->m_stMsg[pstMsgs->m_nMsgNo++]);

		pmsg = strtok_r(NULL, "*", &p);
	}

	return 0;
}

/*
 *sz 未经过预处理
 */
int
Msg_FmMsgs(char sz[MSG_SIZE], stMsgs_t *pstMsgs)
{	

	char *psz = PreprocessMsgs(sz);	//psz <- sz
	Msg_FmMsgs(psz, pstMsgs);
	
	return 0;
}

void
Msg_PrintStMsgs(stMsgs_t stMsg, FILE * fd)
{
	int iMsg, iRow, iWord;
	int nMsg, nRow, nWord;

	fprintf(fd, "%s", "+------------------------------------+\n");
	fprintf(fd, "%s", "|-----------PrintStMsg---------------|\n");
	nMsg = stMsg.m_nMsgNo;
	for (iMsg = 0; iMsg < nMsg; iMsg++) {
		stMsg_t stOneMsg = *(stMsg.m_stMsg[iMsg]);
		puts("<msg>");
		fprintf(fd, "--<time> %d \n--<seq > %d \n--<type> %s %d\n", stOneMsg.m_tRecvTime, stOneMsg.m_nSeq, stOneMsg.m_szType, stOneMsg.m_nType);
		
		nRow = stOneMsg.m_nRowNo;
		for (iRow = 0; iRow < nRow; iRow++) {
			stMsgRow_t stMsgRow = *(stOneMsg.m_pstMsgRow[iRow]);
			if (stMsgRow.m_szWordBegin) {
				fprintf(fd, "----<row> %s \n", stMsgRow.m_szWordBegin);
			}
			nWord = stMsgRow.m_nWordNo;
			for (iWord = 0; iWord < nWord; iWord++) {
				char *word = (stMsgRow.m_szWordRest[iWord]);
				fprintf(fd, "------<word> %s \n", word);
			}
		}
	}
	fprintf(fd, "%s", "|------------------------------------|\n");
}

void
Msg_SaveSzMsgs(char sz[MSG_SIZE], FILE* fd)
{
	stMsgs_t stMsg;

	Msg_FmMsgs(sz, &stMsg);

	Msg_PrintStMsgs(stMsg, fd);
}

















/*
int
TestInform(char sz[MSG_SIZE])
{
	char *psz;

	stMsgs_t stMsg;

	psz = Msg_Preprocess(sz);
	
	puts("\n\n======================================");
	puts(psz);
	
	Msg_FmMsgs(psz, &stMsg);
	
	puts("++++++++++++++++++++++++++++++++++++++");

	Msg_PrintStMsg(stMsg);

	puts("======================================\n\n");
	
	return 0;
}

int
SeveralTest(int argc, int *argv[])
{
	char msg1[MSG_SIZE]="seat/ \nbutton: pid jetton money \nsmall blind: 1111 jetton 100 \nbig blind: 2222 jetton 200 \n3333 jetton 300 \n4444 jetton 400 \n/seat \nseat/ \nbutton: pid jetton money \nsmall blind: 1111 jetton 100 \nbig blind: 2222 jetton 200 \n3333 jetton 300 \n4444 jetton 400 \n/seat \n";
	char msg2[MSG_SIZE]="type1/ \nA1: B1 C1 \n/type1 \ntype2/ \nA2: B2 C2 \nB3 C3 \n/type2 \n";
	char msg3[MSG_SIZE]="type1/ \nA1: B1 | C1 | D1 \n/type1 \n";

	TestInform(msg1);
	TestInform(msg2);
	TestInform(msg3);

	return 0;
}
*/

/*
int main()
{
	char sz[1024] = "seat/button:2222 2000 8000+small blind:1111 2000 8000+big blind:3333 2000 8000+/seat";
	char *psz = sz;
	stMsgs_t stMsgs1;
	FmMsgs(psz, &stMsgs1);
	puts("[------FmMsgs------]");
	return 0;
}
*/
