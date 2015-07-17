#ifndef _LOG_H_
#define _LOG_H_


#define WRITE_COMMFILE 1
#define WRITE_AIFILE   1

extern char g_szCommFile[64];
extern char g_szAiLog[64];

void Log_Init(char suffix[8]);

#endif
