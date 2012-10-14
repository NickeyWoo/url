/*++
 * 
 *	Microblog Writer
 *	Author:	NickWu
 *	Date:	2012-08-13
 *
--*/
#ifndef __LOGS_H__
#define __LOGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NORMAL	0
#define ERROR	1

#define INIT_LOGS(szPath)	logs_init(szPath, &g_stLogInfo)
#define FLUSH_LOGS()		logs_flush(&g_stLogInfo)
#define CLOSE_LOGS()		logs_close(&g_stLogInfo)

#ifdef DEBUG
	#define TRACE_LOG(...)													\
		printf(__VA_ARGS__);												\
		printf("\n")

	#define LOG(...)														\
		printf(__VA_ARGS__);												\
		printf("\n");														\
		logs_write(&g_stLogInfo, NORMAL, __FILE__, __LINE__, __VA_ARGS__)
	
	#define ERROR_LOG(...)													\
		printf(__VA_ARGS__);												\
		printf("\n");														\
		logs_write(&g_stLogInfo, ERROR, __FILE__, __LINE__, __VA_ARGS__)

#else
	#define TRACE_LOG(...)		// trace printf __VA_ARGS__

	#define LOG(...)			logs_write(&g_stLogInfo, NORMAL, __FILE__, __LINE__, __VA_ARGS__)

	#define ERROR_LOG(...)		logs_write(&g_stLogInfo, ERROR, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define MAX_LOG_FILE_SIZE	10485760	// max size is 10MB
#define MAX_LOG_FILE_COUNT	10			// max file count is 10

typedef struct _tag_logs_info {
	FILE* 			pNormalFile;
	FILE* 			pErrorFile;
	char*			szLogPath;
} logs_info;

int logs_init(const char* szLogPath, logs_info* pstLogsInfo);
void logs_flush(logs_info* pstLogsInfo);
void logs_close(logs_info* pstLogsInfo);

int logs_write(logs_info* pstLogsInfo, int iLevel, const char* szFile, int iLine, const char* szFormat, ...) __attribute__ ((format(printf, 5, 6)));

extern logs_info	g_stLogInfo;

#ifdef __cplusplus
}
#endif

#endif // define __LOGS_H__
