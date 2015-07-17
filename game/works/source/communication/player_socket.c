#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "player_socket.h"
#include "msg_process.h"
#include "log.h"

#define DEBUG_STROUT 0

// 前缀 说明					是否使用
// s_ : static				Y
// g_ : global				Y
// p_ : param of function
// c_ : const
// m_ : member of struct
// n  : int					Y
// sz : string				Y
// a  : array				

int g_nClientSock=-1;			// 客户端绑定的套接字
//static char s_szSrvMsg[BUF_SIZE]={0};	// 服务器发来的消息
//static char s_szPlyMsg[BUF_SIZE]={0};	// 客户端

int
Socket_MySend(char sz[BUF_SIZE])
{
	int ret = send(g_nClientSock, sz, strlen(sz) + 1, 0);

	return ret;
}

int
Socket_ReadAndSzToSt(stMsgs_t *pstMsgs)
{
    char szRevBuf[BUF_SIZE] = {'\0'};
    
    int  ret = read(g_nClientSock,szRevBuf,BUF_SIZE);
	//printf("read ret %d\n", ret);
	
	if(ret<0) {
		// 没有接收到正确的内容
		ret = READ_MSG_NO_MSG;
	} else if (0==strcmp(szRevBuf, "game-over \n")) {
		// 接收到结束游戏消息	
		ret = READ_MSG_GAME_OVER;
	} else {		
		//Msg_FmMsgs(szRevBuf, pstMsgs);	//用这个函数出错
		char *psz;
		psz = PreprocessMsgs(szRevBuf);
		//printf("[------PreprocessMsgs------]\n%s\n", psz);
		FmMsgs(psz, pstMsgs);
		//puts("[------FmMsgs------]");
#if WRITE_COMMFILE
		FILE* fdComm = fopen(g_szCommFile, "a");
		fprintf(fdComm, "[++++++rev++++++]\n%s",szRevBuf);
	    Msg_PrintStMsgs(*pstMsgs, fdComm);
	    fclose(fdComm);		
#endif
   	}
   	
	return ret;
}

int 
ConnectServer(char srvIP[14], char srvPt[8], 
				  char plyIP[14], char plyPt[8], 
				  char plyId[8])
{    
	int iSrvPt = atoi(srvPt);
	int iPlyPt = atoi(plyPt);
//	int iPlyId = atoi(plyId);

    int bIsReuseAddr = 1;
    
    struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

#if WRITE_COMMFILE
	FILE* fdComm = fopen(g_szCommFile, "a");
#endif

	//01 创建clientSocket
    g_nClientSock=socket(AF_INET,SOCK_STREAM,0);
    if(g_nClientSock<0) {
#if WRITE_COMMFILE    
        fprintf(fdComm, "socket creation failed\n");
#endif        
        //puts("socket creation failed\n");
        exit(-1);
    }
#if WRITE_COMMFILE    
    fprintf(fdComm, "socket create successfully.\n");
    //puts("socket create successfully.\n");
#endif

    //02 可复用
    setsockopt(g_nClientSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&bIsReuseAddr, sizeof(bIsReuseAddr));
	//02 初始化clientAddr
	bzero(&clientAddr,sizeof(clientAddr));
	clientAddr.sin_family=AF_INET;
	clientAddr.sin_port=htons(iPlyPt);
	clientAddr.sin_addr.s_addr=inet_addr(plyIP);
	//02 客户端绑定端口 bind
    if(bind(g_nClientSock,(struct sockaddr *)&clientAddr,sizeof(struct sockaddr_in))==-1) {
#if WRITE_COMMFILE    
        fprintf(fdComm, "Bind error.\n");
        //puts("Bind error.\n");
#endif        
        return -1;
    }
#if WRITE_COMMFILE    
    fprintf(fdComm, "Bind successful.\n");
    //puts("Bind successful.\n");
#endif
	//03 初始化serverAddr
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons((u_short) iSrvPt);
	serverAddr.sin_addr.s_addr=inet_addr(srvIP);			
	//03 连接到服务器 connect
    while(connect(g_nClientSock,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr_in))<0) {
        usleep(100*1000);
#if WRITE_COMMFILE        
        fprintf(fdComm, "Connect error.\n");
        //puts("Connect error.\n");
#endif        
        //exit(-1);
    }
#if WRITE_COMMFILE    
    fprintf(fdComm, "Connect successful.\n");    
    //puts("Connect successful.\n");
#endif    
	
#if WRITE_COMMFILE
	fclose(fdComm);
#endif

    return 0;
}

void
Socket_SendReg(char plyId[8], char plyName[50])
{
//	usleep(100*1000);
//	int iPlyId = atoi(plyId);	
	char szRegMsg[50] = {'\0'};	
    snprintf(szRegMsg, sizeof(szRegMsg) - 1, "reg: %s %s<%s> \n", plyId, plyName, plyId);
    send(g_nClientSock, szRegMsg, strlen(szRegMsg) + 1, 0);
    
    //puts("Seg successful. \n");
}

void
DisconnectServer()
{
	close(g_nClientSock);	
}





/*
void *
PthreadBasicRead(void *arg) 
{
    char revBuf[BUF_SIZE]="";
    while(1) {    	
    	
        if(read(g_nClientSock,revBuf,BUF_SIZE)==-1) {
            continue;
        }
        if(strlen(revBuf)<=0) {
        	continue;
        }
        
#if WRITE_COMMFILE
        FILE* fdComm = fopen(g_szCommFile, "a");
        fprintf(fdComm, "[++++++rev++++++]\n%s",revBuf);
        Msg_SaveStMsg(revBuf, fdComm);
        fclose(fdComm);        
#endif    
		
        bzero(revBuf,sizeof(revBuf));                
    }
}

void *
PthreadBasicWrite(void *arg)
{
    char sedBuf[BUF_SIZE]="";
    while(1) {
    	if(strlen(sedBuf)<=0) {
    		continue;
    	}    	
        if(write(g_nClientSock,sedBuf,strlen(sedBuf))==-1) {
            continue;
        }        

#if WRITE_COMMFILE
        FILE* fdComm = fopen(g_szCommFile, "a");
        fprintf(fdComm, "[------snd------]\n%s",sedBuf);
        fclose(fdComm);        
#endif
		
        bzero(sedBuf,sizeof(sedBuf));        
    }	
}

char *
Socket_ReadSz()
{
    char *revBuf = (char *)malloc(BUF_SIZE);
    int  ret = -1;	
    
    ret = read(g_nClientSock,revBuf,BUF_SIZE);
    
    if(ret<0) {
    	free(revBuf);
    	return NULL;
    }
       
#if WRITE_COMMFILE
    FILE* fdComm = fopen(g_szCommFile, "a");
    fprintf(fdComm, "[++++++rev++++++]\n%s",revBuf);
    Msg_SaveStMsg(revBuf, fdComm);
    fclose(fdComm);        
#endif

	return revBuf;
}
*/
