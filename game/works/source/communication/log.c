#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


char g_szCommFile[64] = {'\0'};
char g_szAiLog[64] = {'\0'};

void
Log_Init(char suffix[8]) {

	time_t now;
	time(&now);
	
	FILE *fdComm = NULL;
		
	//g_szCommFile
	sprintf(g_szCommFile, "%s%s", "CommFile", suffix);
	remove(g_szCommFile);
		
	fdComm = fopen(g_szCommFile, "w+");
	fprintf(fdComm, "%s\t%s\n\n",g_szCommFile,asctime(localtime(&now)));
	fclose(fdComm);
	
	//g_szAiLog
	sprintf(g_szAiLog, "%s%s", "AiLog", suffix);
	remove(g_szAiLog);
	
	fdComm = fopen(g_szAiLog, "w+");
	fprintf(fdComm, "%s\t%s\n\n",g_szAiLog,asctime(localtime(&now)));
	fclose(fdComm);
	
	//printf("Log_Init finished.\n");
	
}
