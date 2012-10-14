/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#ifndef __MISC_H__
#define __MISC_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

void HexDump(const char* pBuffer, int iLen, char** pOutBuffer);

#define OCT_TYPE 8
#define DEC_TYPE 10
#define HEX_TYPE 16

int number_printf_size(long long int llNum, int iType);

int GetRuntimePath(char* pBuffer, int iLen);
int CreatePidFile(const char* szPidFile);

int CreateDir(const char* szDirPath, mode_t mode);

int GetDomainAddr(const char* szDomain, struct in_addr* pstAddr);

u_int64_t MD5_uint64(const char* pBuffer, int iLen);
u_int64_t CityHash_uint64(const char* pBuffer, int iLen);

#define HASH(buffer, len)		CityHash_uint64(buffer, len)

u_int32_t GetPrimeNumber(u_int32_t dwMaxNum);
int GetPrimeNumberArray(u_int32_t dwMaxNum, u_int32_t* pPrimeNumBuffer, size_t sz);

#ifdef __cplusplus
}
#endif

#endif // define __MISC_H__
