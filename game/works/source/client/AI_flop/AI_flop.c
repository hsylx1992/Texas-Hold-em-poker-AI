
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "player_socket.h"
#include "msg_process.h"

#include "log.h"

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

	printf("Connect Info:\n");
	printf("Server:%s(%s)\n", strServerIP, strServerSocketNo);
	printf("Client:%s(%s)\n", strPlayerIP, strPlayerSocketNo);

	Log_Init(strPlayerID);			// Log文件初始化	
	ConnectServer(strServerIP, strServerSocketNo, 
				  strPlayerIP, strPlayerSocketNo, 
				  strPlayerID);		// 连接服务器
	Socket_SendReg(strPlayerID, "fold");

	while (1) {		
		stMsgs_t stMsgs;	
		int ret;
		bzero(&stMsgs, sizeof(stMsgs));
		
		ret = Socket_ReadAndSzToSt(&stMsgs);
		
		if (READ_MSG_NO_MSG == ret) {
			usleep(10*1000);
			continue;
		} else if (READ_MSG_GAME_OVER == ret) {
			break;
		}
				
		#if 1
		for (int i = 0; i < stMsgs.m_nMsgNo; ++i) {
			stMsg_t stOneMsg = *(stMsgs.m_stMsg[i]);

			puts(stOneMsg.m_szType);
	
			if (0==strcmp(stOneMsg.m_szType, "inquire")) {
				char szActionMsg[50] = "fold \n"; 
				int  ret = Socket_MySend(szActionMsg);
				printf("------%d------%s------\n", ret, szActionMsg);
			}
		}
		#endif

		usleep(10*1000);
	}

	DisconnectServer();				// 断开服务器

	return 0;
}


