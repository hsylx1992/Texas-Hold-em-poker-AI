
#ifndef _PLAYER_SOCKET_H_
#define _PLAYER_SOCKET_H_

#include "msg_process.h"

#ifdef  MSG_SIZE
#define BUF_SIZE MSG_SIZE
#else	
#define BUF_SIZE 4096
#endif

#define	READ_MSG_GAME_OVER 9999
#define	READ_MSG_NO_MSG -1

void *PthreadBasicRead(void *arg);
void *PthreadBasicWrite(void *arg);
void LoopCommPthread();

int  ConnectServer(char srvIP[14], char srvPt[8], char plyIP[14], char plyPt[8], char plyId[8]);
void DisconnectServer();

int  Socket_ReadAndSzToSt(stMsgs_t *pstMsgs);
int  Socket_MySend(char sz[BUF_SIZE]);

void Socket_SendReg(char plyId[8], char plyName[50]);

extern int g_nClientSock;


#endif
