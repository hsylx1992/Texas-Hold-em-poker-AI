
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "player_socket.h"
#include "msg_process.h"

#include "log.h"
#include "eval.h"
#include "pokerai.h"

#define MAX_TRIES 5
#define TIMEOUT   300
#define PRINTERR(...) fprintf(stderr, __VA_ARGS__)

#define DEBUG_SIMUTIME 0

static int s_bIsNeedSend = 0;
/*
 * @p 		srvIP 		srvPt	playerIP 	plyPt plyId
 * ./game   127.0.0.1 	6000    127.0.0.2   6002  6001 
 */
int main(int argc, char* argv[])
{
	char strPlayerIP[16], strServerIP[16];
	char strPlayerSocketNo[8], strServerSocketNo[8];
	char strPlayerID[8];

	if (1 >= argc) {
		strcpy(strServerIP, "127.0.0.1");
		strcpy(strServerSocketNo, "6000");
		strcpy(strPlayerIP, "127.0.0.2");
		strcpy(strPlayerSocketNo, "6002");
		strcpy(strPlayerID, "2222");
	} else {
		strcpy(strServerIP, argv[1]);
		strcpy(strServerSocketNo, argv[2]);
		strcpy(strPlayerIP, argv[3]);
		strcpy(strPlayerSocketNo, argv[4]);
		strcpy(strPlayerID, argv[5]);
	}

	//printf("Connect Info:\n");
	//printf("Server:%s(%s)\n", strServerIP, strServerSocketNo);
	//printf("Client:%s(%s)\n", strPlayerIP, strPlayerSocketNo);
	
//===============================================================
	PokerAI  *AI = NULL;
  	char *action = NULL;
  	//char *handranksfile = "/home/hsy/HandRanks.dat";
    
	Log_Init(strPlayerID);			// Log文件初始化
  	//InitEvaluator(handranksfile);	// Evaluator初始化	
	ConnectServer(strServerIP, strServerSocketNo, 
				  strPlayerIP, strPlayerSocketNo, 
				  strPlayerID);		// 连接服务器
	Socket_SendReg(strPlayerID, "game");
	
	AI = CreatePokerAI(TIMEOUT);	
#if WRITE_AIFILE	
	FILE *fdAiLog = fopen(g_szAiLog, "a");
	SetLogging(AI, LOGLEVEL_DEBUG, fdAiLog);
#endif  	
	strcpy(AI->game.name, strPlayerID);
	//AI->game.money = 4000;
	//AI->game.BB    = 40;
	
	// msg_process中考虑到粘包情况，这里认为是没有粘包的(v1.24更新)
	while (1) {
		stMsgs_t stMsgs;	
		int ret;
		bzero(&stMsgs, sizeof(stMsgs));
		
		ret = Socket_ReadAndSzToSt(&stMsgs);
		
		if (READ_MSG_NO_MSG == ret) {
			//usleep(10*1000);
			continue;
		} else if (READ_MSG_GAME_OVER == ret) {
			break;
		}

#if DEBUG_SIMUTIME		
struct timeval tpstart,tpend;     
float timeuse;     
gettimeofday(&tpstart,NULL); // 开始时间
#endif		
		for (int i = 0; i < stMsgs.m_nMsgNo; ++i) {
			stMsg_t stOneMsg = *(stMsgs.m_stMsg[i]);

			//puts(stOneMsg.m_szType);
			
			UpdateGameState(AI, stOneMsg);
#if WRITE_COMMFILE			
			FILE* fdComm = fopen(g_szCommFile, "a");						
			PrintTableInfo(&AI->game, fdComm);
			fclose(fdComm);	
#endif
		    //if (MyTurn(AI))
		    if (0==strcmp(stOneMsg.m_szType, "inquire"))
		    {
		        int attempts = 0;
				int retsend = -1;				
				
				s_bIsNeedSend = 1;
				
		        //Run Monte Carlo simulations to determine the best action
		        
		        action = GetBestAction(AI);
		        		        
		        //sprintf(postURL, "%s%s", POST_URL, action);
		        //WriteAction(AI, stdout);

		        //Post the action to the server
		        while (attempts < MAX_TRIES && retsend < 0)
		        {
		            retsend = Socket_MySend(action);
		            attempts++;

		            if (retsend < 0) {
		                PRINTERR("Could not SEND response (attempt %d)\n", attempts);
		            }
		            
		        }

		        if (attempts == MAX_TRIES){
		            PRINTERR("Was not able to SEND!\n");
		        }
		        
		        if (retsend >=0) {
		        	s_bIsNeedSend = 0;
		        }
		        
		    }
        
		} // for each msg in msgs
#if DEBUG_SIMUTIME
// 计算执行时间，以微秒为单位进行计算 
gettimeofday(&tpend,NULL);     // 结束时间     
timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;       
timeuse/=1000000;
if (timeuse >= 0.100001) {
	printf("used time:%f sec\n",timeuse);
}
#endif		

	}

	DisconnectServer();				// 断开服务器
//	DestroyPokerAI(AI);				// 释放资源

	return 0;
}


