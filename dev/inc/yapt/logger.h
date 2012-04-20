/*-------------------------------------------------------------------------
File    : Logger.cpp
Author  : FKling
Orginal : 2005-7-19, 11:39
Descr   : Logger, simplistic stuff, mimics Log4Net/Log4Java/Log4Delphi

--------------------------------------------------------------------------- 
Todo [-:undone,+:inprogress,!:done]:
 - Consider removing the namespace, however, quite a few classes in here, would like to skip prefixing

Changes: 

-- Date -- | -- Name ------- | -- Did what...                              
2009-10-28 | FKling          | Back to yapt workspace
2009-04-16 | FKling          | Imported from other projects
2009-01-26 | FKling          | Imported to iPhone
2006-11-08 | FKling          | Rewrote critical section handling, now specific lock's
2006-10-19 | FKling          | Timers are multi-core, multi-cpu safe
2006-01-12 | FKling          | upon creation of timers calling mark, to reset states
2005-12-12 | FKling          | memory leak, free generated debugstring

---------------------------------------------------------------------------*/
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <list>

using namespace std;

namespace yapt
{
#define MAX_INDENT 256
	class ILogger
	{
	public:
		// properties
		virtual int GetIndent() = 0;
		virtual int SetIndent(int nIndent) = 0;
		virtual char *GetName() = 0;
		
		// functions
		virtual void WriteLine(const char *sFormat,...) = 0;
		virtual void Critical(const char *sFormat,...) = 0;
		virtual void Error(const char *sFormat, ...) = 0;
		virtual void Warning(const char *sFormat, ...) = 0;
		virtual void Info(const char *sFormat, ...) = 0;
		virtual void Debug(const char *sFormat, ...) = 0;
		
		virtual void Enter() = 0;
		virtual void Leave() = 0;
	};
	// Holds properties for the sink
	class LogSinkProperties
	{
	public:
		int iDebugLevel;	// Everything above this becomes written to the sink
	public:
		LogSinkProperties();
	};
	class ILogOutputSink
	{
	public:
		virtual void Initialize(int argc, char **argv) = 0;
		virtual void WriteLine(int dbgLevel, char *hdr, char *string) = 0;
		virtual void Close() = 0;
	};
	class LogBaseSink : public ILogOutputSink
	{
	protected:
		LogSinkProperties properties;
		__inline bool WithinRange(int iDbgLevel) { return (iDbgLevel>=properties.iDebugLevel)?true:false; }
	public:	
		LogSinkProperties *GetProperties() { return &properties; }
	public:
		virtual void Initialize(int argc, char **argv) = 0;
		virtual void WriteLine(int dbgLevel, char *hdr, char *string) = 0;
		virtual void Close() = 0;
	};
	class LogConsoleSink : 	public LogBaseSink
	{
	public:
		virtual void Initialize(int argc, char **argv);
		virtual void WriteLine(int dbgLevel, char *hdr, char *string);
		virtual void Close();
	};
	
	class LogFileSink : public LogBaseSink
	{
	private:
		FILE *fOut;
		char *sFileName;
	public:
		virtual void Initialize(int argc, char **argv);
		virtual void WriteLine(int dbgLevel, char *hdr, char *string);
		virtual void Close();
	};
	
	class LoggerInstance
	{
	public:
		ILogger *pLogger;
		int iDebugLevel;		// Message class must be above this to become printed
		std::list<char *> lExcludedModeuls;		
	public:
		LoggerInstance();
		LoggerInstance(ILogger *pLogger, int iDebugLevel);
	};

	typedef std::list<LoggerInstance *> ILoggerList;
	typedef std::list<ILogOutputSink *>ILoggerSinkList;

	class Logger : public ILogger
	{
	public:
		typedef enum
		{
			kMCNone = 0,
			kMCDebug,
			kMCInfo,
			kMCWarning,
			kMCError,
			kMCCritical,
		} MessageClass;
		
		typedef enum
		{
			kTFDefault,
			kTFLog4Net,
			kTFUnix,			
		} TimeFormat;
	private:
		
		char *sName;
		char *sIndent;
		int iIndentLevel;
		Logger(const char *sName);
		void WriteReportString(MessageClass mc, char *string);
		void GenerateIndentString();
		
	private:
		static TimeFormat kTimeFormat;
		static bool bInitialized;
		static int iIndentStep;
		static ILoggerList loggers;
		static ILoggerSinkList sinks;
		static LoggerInstance defaultInstanceSettings;
		static char *TimeString(int maxchar, char *dst);
		static void SendToSinks(int dbgLevel, char *hdr, char *string);
		static void Initialize();
	public:
	
		virtual ~Logger();
		static ILogger *GetLogger(const char *name);
		static void SetAllSinkDebugLevel(int iNewDebugLevel);
		
		
		// properties
		virtual int GetIndent() { return iIndentLevel; };
		virtual int SetIndent(int nIndent) { iIndentLevel = nIndent; return iIndentLevel; };
		virtual char *GetName() { return sName;};
		
		// Functions
		virtual void WriteLine(const char *sFormat,...);
		virtual void Critical(const char *sFormat,...);
		virtual void Error(const char *sFormat, ...);
		virtual void Warning(const char *sFormat, ...);
		virtual void Info(const char *sFormat, ...);
		virtual void Debug(const char *sFormat, ...);
		
		virtual void Enter();
		virtual void Leave();
	};
	
}
