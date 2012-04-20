/*-------------------------------------------------------------------------
 File    : $Archive: $
 Author  : $Author: $
 Version : $Revision: $
 Orginal : 2006-07-26, 15:50
 Descr   : Tiny Log4Net look-alike for C++ 
 
 Comparision in output is '>=' means, setting debug level filtering to 
 INFO gives INFO and everything above..
 
 Levels:
 0 - none,
 1 - debug,
 2 - info,
 3 - warning
 4 - error
 5 - crtical
 
 Modified: $Date: $ by $Author: $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
   ! Level filtering [sink levels]
   - Support for module exclusion/inclusion lists
   - ? Introduce more debug levels to reduce noise
 </pre>
 
 
 \History
 - 02.04.2009, FKling, Support for sink levels
 - 23.03.2009, FKling, Implementation
 
 ---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#define snprintf _snprintf
#define vsnprintf _vsnprintf

#else
#include <pthread.h>
#include <sys/time.h>
#endif

#include <list>


#include "yapt/logger.h"

#define DEFAULT_DEBUG_LEVEL 0		// used by constructors, default is output everything


using namespace yapt;
using namespace std;

///////////////////
//
// Implementation of the console sink
// outputs all debug messages to the console
//
void LogConsoleSink::Initialize(int argc, char **argv)
{
	// Do stuff here
	int i;
	for (i=0;i<argc;i++)
	{
		if (!strcmp(argv[i],"filename"))
		{
			// 
		}
	}
}
void LogConsoleSink::WriteLine(int dbgLevel, char *hdr, char *string)
{
	if (WithinRange(dbgLevel))
	{
		fprintf(stdout, "%s%s\n", hdr,string);
	}
}
void LogConsoleSink::Close()
{
	// close file here
}


void LogFileSink::Initialize(int argc, char **argv)
{
	int i;
	sFileName = NULL;
	fOut = NULL;
	
	for (i=0;i<argc;i++)
	{
		if (!strcmp(argv[i],"file"))
		{
			sFileName = strdup(argv[++i]);
		}
	}
	
	if (sFileName == NULL)
	{
		return;
	}
	fOut = fopen(sFileName,"w");
	
}
void LogFileSink::WriteLine(int dbgLevel, char *hdr, char *string)
{
	if (fOut != NULL)
	{
		if (WithinRange(dbgLevel))
		{
			fprintf(fOut,"%s%s",hdr,string);
		}
	}
}
void LogFileSink::Close()
{
	if (fOut != NULL)
	{
		fclose(fOut);
	}
}

/////////
//
// -- static functions
//
int Logger::iIndentStep = 2;
bool Logger::bInitialized = false;
ILoggerList Logger::loggers;
ILoggerSinkList Logger::sinks;
Logger::TimeFormat Logger::kTimeFormat = kTFLog4Net;
LoggerInstance Logger::defaultInstanceSettings;

void Logger::SendToSinks(int dbgLevel, char *hdr, char *string)
{
	ILogOutputSink *pSink = NULL;
	ILoggerSinkList::iterator it;
	it = sinks.begin();
	while(it != sinks.end())
	{
		pSink = (ILogOutputSink *)*it;
		pSink->WriteLine(dbgLevel, hdr, string);
		it++;
	}
}


#ifdef WIN32
struct timezone 
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

static int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS; 
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
	return 0;
}
#endif
//
// Returns a formatted time string for logging
// string can be either in default kTFLog4Net format or Unix
//
char *Logger::TimeString(int maxchar, char *dst)
{
	struct timeval tmv;
	gettimeofday(&tmv, NULL);
	
	switch(kTimeFormat)
	{
		case kTFDefault :
		case kTFUnix :
#ifdef WIN32
			//ctime_s(&tmv.tv_sec, 24,dst);
#else
			ctime_r(&tmv.tv_sec, dst);
#endif
			dst[24] = '\0';
			break;
		case kTFLog4Net :
			{
				struct tm *gmt = gmtime(&tmv.tv_sec);
				snprintf(dst,maxchar,"%.2d.%.2d.%.4d %.2d:%.2d:%.2d.%.3d",
						 gmt->tm_mday,gmt->tm_mon,gmt->tm_year+1900, 
						 gmt->tm_hour,gmt->tm_min,gmt->tm_sec,tmv.tv_usec/1000);
			}
			break;
			
	}
	return dst;
}


ILogger *Logger::GetLogger(const char *name)
{
	ILogger *pLogger = NULL;
	LoggerInstance *pInstance;
	ILoggerList::iterator it;
	
	Initialize();
	
	it = loggers.begin();
	while(it != loggers.end())
	{
		pInstance = (LoggerInstance *)*it;
		pLogger = pInstance->pLogger;
		if (!strcmp(pLogger->GetName(), name))
		{
			return pLogger;
		}
		it++;
	}
	
	// Have to create a new logger
	pLogger = (ILogger *)new Logger(name);
	pInstance = new LoggerInstance(pLogger, defaultInstanceSettings.iDebugLevel);
	// TODO: Support for exclude list
	
	loggers.push_back(pInstance);
	return pLogger;
	
}
void Logger::SetAllSinkDebugLevel(int iNewDebugLevel)
{
	// This might very well be the first call, make sure we are initalized
	Initialize();
	
	LogBaseSink *pSink = NULL;
	ILoggerSinkList::iterator it;
	it = sinks.begin();
	while(it != sinks.end())
	{
		pSink = (LogBaseSink *)*it;
		pSink->GetProperties()->iDebugLevel = iNewDebugLevel;
		it++;
	}
}


void Logger::Initialize()
{
	if (!Logger::bInitialized)
	{
		// Set defaults
		defaultInstanceSettings.pLogger = NULL;
		defaultInstanceSettings.iDebugLevel = -1;	// - all messages written by default
		
		
		// Initialize the rest
		ILogOutputSink *pSink = (ILogOutputSink *)new LogConsoleSink();
		pSink->Initialize(0, NULL);
		sinks.push_back(pSink);
		Logger::bInitialized = true;
	}
}

// Regular functions

Logger::Logger(const char *sName)
{
	this->sName = strdup(sName);
	this->iIndentLevel = 0;
	this->sIndent = (char *)malloc(MAX_INDENT+1);
	memset(this->sIndent,0,MAX_INDENT+1);
	Logger::Initialize();
}
Logger::~Logger()
{
	free(this->sName);
	// remove this from list of loggers
}
#define MAX_CLASSES 5
static char *lClassNames[MAX_CLASSES+1] = 
{
	"",				// 0
	"DEBUG",		// 1
	"INFO",			// 2
	"WARN",			// 3
	"ERROR",		// 4
	"CRITICAL"		// 5
};

void Logger::WriteReportString(MessageClass mc, char *string)
{
	char sHdr[MAX_INDENT + 64];
	char sTime[32];	// saftey, 26 is enough
	int iClass = (int)mc;

	if (iClass > MAX_CLASSES) 
	{
		iClass = MAX_CLASSES;
	}
	char *sLevel = lClassNames[iClass];

	
	TimeString(32, sTime);
	// Create the special header string
	// Format: "time [thread] msglevel module - "

#ifdef WIN32
	DWORD tid = 0;
	//tid = GetCurrentThreadID();	
	snprintf(sHdr, MAX_INDENT + 64, "%s [%.8x] %8s %32s - %s", sTime, tid, sLevel, sName, sIndent);
#else
	void *tid = NULL;
	tid = pthread_self();	
	snprintf(sHdr, MAX_INDENT + 64, "%s [%p] %8s %32s - %s", sTime, tid, sLevel, sName, sIndent);
#endif
	Logger::SendToSinks((int)mc,sHdr, string);
}


void Logger::GenerateIndentString()
{
	memset(this->sIndent,0,MAX_INDENT+1);
	memset(this->sIndent,' ',iIndentLevel);
}


#define INITIALIZE_FORMAT_STRING \
va_list	values;			\
char * newstr = NULL;	\
size_t BufferSize=1024;	\

#define GENERATE_FORMAT_STRING(__str__) \
do												\
{												\
newstr=(char *)realloc(newstr,BufferSize);		\
va_start( values, (__str__) );					\
vsnprintf(newstr, BufferSize, __str__, values); \
va_end(	values);								\
if (strlen(newstr)>=BufferSize)					\
{												\
BufferSize=BufferSize+BufferSize;				\
} else											\
{												\
BufferSize=0;									\
}												\
} while(BufferSize!=0);

#define FINALIZE_FORMAT_STRING	free(newstr);

//
// TODO: If this works fine, fill in Error/Warning/Info/Debug
//
void Logger::WriteLine(const char *sFormat,...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCNone, newstr);
	
	FINALIZE_FORMAT_STRING
}
void Logger::Critical(const char *sFormat,...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCCritical, newstr);
	
	FINALIZE_FORMAT_STRING
}
void Logger::Error(const char *sFormat, ...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCError, newstr);
	
	FINALIZE_FORMAT_STRING
}
void Logger::Warning(const char *sFormat, ...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCWarning, newstr);
	
	FINALIZE_FORMAT_STRING
}
void Logger::Info(const char *sFormat, ...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCInfo, newstr);
	
	FINALIZE_FORMAT_STRING
}
void Logger::Debug(const char *sFormat, ...)
{
	INITIALIZE_FORMAT_STRING
	GENERATE_FORMAT_STRING(sFormat)
	
	Logger::WriteReportString(kMCDebug, newstr);
	
	FINALIZE_FORMAT_STRING
}

void Logger::Enter()
{
	iIndentLevel+=Logger::iIndentStep;
	if (iIndentLevel > MAX_INDENT)
	{
		iIndentLevel = MAX_INDENT;
	}
	GenerateIndentString();
}
void Logger::Leave()
{
	iIndentLevel-=Logger::iIndentStep;
	if (iIndentLevel < 0)
	{
		iIndentLevel = 0;
	}
	GenerateIndentString();
}

/////////////////
LogSinkProperties::LogSinkProperties()
{
	this->iDebugLevel = DEFAULT_DEBUG_LEVEL;
}

LoggerInstance::LoggerInstance()
{
	this->pLogger = NULL;
	this->iDebugLevel = DEFAULT_DEBUG_LEVEL;	
}
LoggerInstance::LoggerInstance(ILogger *pLogger, int iDebugLevel)
{
	this->pLogger = pLogger;
	this->iDebugLevel = iDebugLevel;
}






