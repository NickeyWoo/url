/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include "logs.h"

logs_info	g_stLogInfo;

extern int shift_logfiles(FILE** ppFile, const char* szLogPath, const char* szFileName);

int logs_init(const char* szLogPath, logs_info* pstLogsInfo)
{
	if(pstLogsInfo == NULL)
		return -1;

	char szBuffer[260];
	memset(szBuffer, 0, 260);
	
	sprintf(szBuffer, "%s/normal_0.log", szLogPath);
	pstLogsInfo->pNormalFile= fopen(szBuffer, "a+");
	if(pstLogsInfo->pNormalFile== NULL)
	{
		return -1;
	}

	memset(szBuffer, 0, 260);
	sprintf(szBuffer, "%s/error_0.log", szLogPath);
	pstLogsInfo->pErrorFile = fopen(szBuffer, "a+");
	if(pstLogsInfo->pErrorFile == NULL)
	{
		fclose(pstLogsInfo->pNormalFile);
		return -1;
	}

	int iPathLen = strlen(szLogPath) + 1;
	pstLogsInfo->szLogPath = (char*)malloc(iPathLen);
	memset(pstLogsInfo->szLogPath, 0, iPathLen);
	strcpy(pstLogsInfo->szLogPath, szLogPath);
	return 0;
}

void logs_flush(logs_info* pstLogsInfo)
{
	if(pstLogsInfo == NULL)
		return;

	fflush(pstLogsInfo->pNormalFile);
	fflush(pstLogsInfo->pErrorFile);
}

void logs_close(logs_info* pstLogsInfo)
{
	if(pstLogsInfo == NULL)
		return;

	fflush(pstLogsInfo->pNormalFile);
	fflush(pstLogsInfo->pErrorFile);

	fclose(pstLogsInfo->pNormalFile);
	fclose(pstLogsInfo->pErrorFile);
	
	free(pstLogsInfo->szLogPath);
}

int logs_write(logs_info* pstLogsInfo, int iLevel, const char* szFile, int iLine, const char* szFormat, ...)
{
	va_list ap;
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm now;
	localtime_r(&tv.tv_sec, &now);
	
	va_start(ap, szFormat);
	if(iLevel == ERROR)
	{
		fprintf(pstLogsInfo->pErrorFile, "[%04d-%02d-%02d %02d:%02d:%02d.%.6ld][%s:%d] ", now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, tv.tv_usec, szFile, iLine);
		vfprintf(pstLogsInfo->pErrorFile, szFormat, ap);
		fprintf(pstLogsInfo->pErrorFile, "\n");
		va_end(ap);
		return shift_logfiles(&(pstLogsInfo->pErrorFile), pstLogsInfo->szLogPath, "error");
	}
	else
	{
		fprintf(pstLogsInfo->pNormalFile, "[%04d-%02d-%02d %02d:%02d:%02d.%.6ld][%s:%d] ", now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, tv.tv_usec, szFile, iLine);
		vfprintf(pstLogsInfo->pNormalFile, szFormat, ap);
		fprintf(pstLogsInfo->pNormalFile, "\n");
		va_end(ap);
		return shift_logfiles(&(pstLogsInfo->pNormalFile), pstLogsInfo->szLogPath, "normal");
	}
}

int shift_logfiles(FILE** ppFile, const char* szLogPath, const char* szFileName)
{
	if(ppFile == NULL ||
		*ppFile == NULL ||
		szLogPath == NULL)
		return -1;

	int fd = fileno(*ppFile);
	if(fd == -1)
		return -1;
	
	struct stat stStat;
	if(-1 == fstat(fd, &stStat))
		return -1;
	
	if(stStat.st_size >= MAX_LOG_FILE_SIZE)
	{
		fflush(*ppFile);
		fclose(*ppFile);

		char szFilePathBuffer[260];
		for(int i=MAX_LOG_FILE_COUNT-1; i>=0; --i)
		{
			memset(szFilePathBuffer, 0, 260);
			sprintf(szFilePathBuffer, "%s/%s_%d.log", szLogPath, szFileName, i);
			
			if(0 == access(szFilePathBuffer, O_RDWR))
			{
				if(i == (MAX_LOG_FILE_COUNT - 1))
					unlink(szFilePathBuffer);
				else
				{
					char szNewFilePathBuffer[260];
					memset(szNewFilePathBuffer, 0, 260);
					sprintf(szNewFilePathBuffer, "%s/%s_%d.log", szLogPath, szFileName, i+1);
					rename(szFilePathBuffer, szNewFilePathBuffer);
				}
			}
		}
		*ppFile = fopen(szFilePathBuffer, "a+");
	}
	return 0;
}

