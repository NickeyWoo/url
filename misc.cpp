/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <openssl/md5.h>
#include <math.h>

// google cityhash
#include <city.h>

#include "misc.h"
#include "logs.h"
#include "oi_str2.h"

#define HEXDUMP_PRINTF(...)														\
			if(pOutBuffer != NULL)												\
			{																	\
				memset(szLineBuffer, 0, 100);									\
				snprintf(szLineBuffer, 100, __VA_ARGS__);						\
				int iLineSize = strlen(szLineBuffer);							\
				snprintf(pCurOutBuffer, iCurOutBufferLen, "%s", szLineBuffer);	\
				pCurOutBuffer += iLineSize;										\
				iCurOutBufferLen -= iLineSize;									\
			}																	\
			else																\
				printf(__VA_ARGS__)

int g_fdPidFile = -1;

void HexDump(const char* pBuffer, int iLen, char** pOutBuffer)
{
	if(pBuffer == NULL || iLen == 0)
		return;

	int iLines = iLen / 16;
	int iMod = iLen % 16;
	if(iMod != 0)
		++iLines;

	int iLeftAlign = number_printf_size(iLines, HEX_TYPE);
	char szFormat[32];
	memset(szFormat, 0, 32);
	sprintf(szFormat, "%%0%dX0 | ", iLeftAlign);

	char szLineBuffer[100];
	char* pCurOutBuffer = NULL;
	int iOutBufferLen = 0;
	int iCurOutBufferLen = 0;
	if(pOutBuffer != NULL)
	{
		iOutBufferLen = (iLeftAlign + 71) * (iLines + 1) + 1;
		*pOutBuffer = (char*)malloc(iOutBufferLen);
		pCurOutBuffer = *pOutBuffer;
		iCurOutBufferLen = iOutBufferLen;
		memset(*pOutBuffer, 0, iOutBufferLen);
	}

	for(int i=0; i<iLeftAlign; ++i)
		HEXDUMP_PRINTF("_");
	HEXDUMP_PRINTF("__|__0__1__2__3__4__5__6__7__8__9__A__B__C__D__E__F_|_________________\n");

	int iLineNum = 0;
	HEXDUMP_PRINTF(szFormat, iLineNum);

	char szStrBuffer[17];
	memset(szStrBuffer, 0, 17);
	for(int i=0; i<iLen; ++i)
	{
		unsigned char c = (unsigned char)pBuffer[i];
		HEXDUMP_PRINTF("%02X ", c);
	
		int idx = i % 16;
		if(c > 31 && c < 127)
			szStrBuffer[idx] = c;
		else
			szStrBuffer[idx] = '.';

		if((i + 1) % 16 == 0)
		{
			++iLineNum;
			HEXDUMP_PRINTF("| %s\n", szStrBuffer);
			if((i + 1) >= iLen)
				break;
			HEXDUMP_PRINTF(szFormat, iLineNum);
			memset(szStrBuffer, 0, 17);
		}
	}
	if(iMod)
	{
		int iLastNum = 16 - iMod;
		for(int i=0; i<iLastNum; ++i)
		{
			HEXDUMP_PRINTF("   ");
		}
		HEXDUMP_PRINTF("| %s\n", szStrBuffer);
	}
}

int number_printf_size(long long int llNum, int iType)
{
	char buffer[23];
	memset(buffer, 0, 23);

	if(iType == OCT_TYPE)
	{
		sprintf(buffer, "%llo", llNum);
		return strlen(buffer);
	}
	else if(iType == DEC_TYPE)
	{
		sprintf(buffer, "%lld", llNum);
		return strlen(buffer);
	}
	else if(iType == HEX_TYPE)
	{
		sprintf(buffer, "%llx", llNum);
		return strlen(buffer);
	}
	return 0;
}

int GetRuntimePath(char* pBuffer, int iLen)
{
	if(pBuffer == NULL || iLen == 0)
		return 0;

	ssize_t size = readlink("/proc/self/exe", pBuffer, iLen);
	if(size == -1)
		return -1;

	char* pLastDir = strrchr(pBuffer, '/');
	pLastDir[1] = 0;
	return strlen(pBuffer);
}

int CreatePidFile(const char* szPidFile)
{
	g_fdPidFile = open(szPidFile, O_RDWR|O_CREAT, 0640);
	if(g_fdPidFile == -1)
		return -1;

	struct flock stLock;
	memset(&stLock, 0, sizeof(struct flock));
	stLock.l_type = F_WRLCK;

	if(fcntl(g_fdPidFile, F_SETLK, &stLock) == -1)
		return -1;

	return 0;
}

int CreateDir(const char* szDirPath, mode_t mode)
{
	int iLen = strlen(szDirPath);
	char* pBuffer = (char*)malloc(iLen + 2);
	memset(pBuffer, 0, iLen + 2);
	strcpy(pBuffer, szDirPath);

	if(pBuffer[iLen - 1] != '/')
	{
		pBuffer[iLen] = '/';
		++iLen;
	}

	for(int i=1; i<iLen; ++i)
	{
		if(pBuffer[i] == '/')
		{
			pBuffer[i] = 0;
			if(access(pBuffer, F_OK) != 0)
			{
				if(mkdir(pBuffer, mode) == -1)
				{
					free(pBuffer);
					return -1;
				}
			}
			pBuffer[i] = '/';
		}
	}

	free(pBuffer);
	return 0;
}

int GetDomainAddr(const char* szDomain, struct in_addr* pstAddr)
{
	if(szDomain == NULL || pstAddr == NULL)
		return -1;

	struct hostent* pstHost = gethostbyname(szDomain);
	if(pstHost == NULL)
		return -1;

	memcpy(pstAddr, pstHost->h_addr, sizeof(struct in_addr));
	return 0;
}

u_int64_t MD5_uint64(const char* pBuffer, int iLen)
{
	MD5_CTX stCtx;
	if(MD5_Init(&stCtx) != 1)
		return 0;
	
	if(1 != MD5_Update(&stCtx, pBuffer, iLen))
		return 0;
	
	unsigned char cHashCodeBuffer[16];
	memset(cHashCodeBuffer, 0, 16);
	if(1 != MD5_Final(cHashCodeBuffer, &stCtx))
		return 0;

	return ntohll(*((u_int64_t*)(cHashCodeBuffer + 4)));
}

u_int64_t CityHash_uint64(const char* pBuffer, int iLen)
{
	return CityHash64(pBuffer, iLen);
}

u_int32_t GetPrimeNumber(u_int32_t dwMaxNum)
{
	for(u_int32_t prime=dwMaxNum; prime>=2; --prime)
	{
		u_int32_t mod = 1;
		u_int32_t prime_sqrt = sqrt(prime);
		for(u_int32_t i=2; i<=prime_sqrt; ++i)
		{
			mod = prime % i;
			if(!mod)
				break;
		}
		if(mod)
			return prime;
	}
	return 0;
}

int GetPrimeNumberArray(u_int32_t dwMaxNum, u_int32_t* pPrimeNumBuffer, size_t sz)
{
	u_int32_t i = 0;
	for(u_int32_t prime = dwMaxNum;
		prime >= 2 && i < sz;
		--prime, ++i)
	{
		prime = GetPrimeNumber(prime);
		if(prime == 0)
			break;
		pPrimeNumBuffer[i] = prime;
	}
	return i;
}




