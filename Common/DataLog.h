#pragma once


#include <sys\types.h>
#include "MutexControl.h"


class BinaryFileWriter;


class DataLog
{
public:
	enum LogLevel
	{
		LogNone = 0,
		LogForced = LogNone,
		LogError,
		LogWarning,
		LogBasicInfo,
		LogSummary,
		LogDetails,
		LogVerbose,
		LogVeryVerbose
	};

	DataLog(void);
	virtual ~DataLog(void);
	void SetLogLevel(LogLevel logLevel);
	bool IsEffectiveLogLevel(LogLevel logLevel);
	bool SetLogFilePath(TCHAR* szLogFilePath);
	bool SetLogFileNameInTemp(TCHAR* szLogFileName);
	int  Close(void);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    message);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		int      integerValue);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		int      integerValue1,
		int      integerValue2);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		int      integerValue1,
		int      integerValue2,
		int      integerValue3);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		int      integerValue,
		double   doubleValue);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		void*    pointerValue);

	int LogEvent(
		LogLevel severityLevel,
		char*    eventSubject,
		char*    messageFormatString,
		void*    pointerValue,
		int      integerValue);

	int LogMessage(
		LogLevel severityLevel,
		char*    message);

	int LogMessage(
		LogLevel severityLevel,
		char*    messageFormatString,
		int      integerValue);

	int LogMessage(
		LogLevel severityLevel,
		char*    messageFormatString,
		int      integerValue1,
		int      integerValue2);

	int LogMessage(
		LogLevel severityLevel,
		char*    messageFormatString,
		int      integerValue1,
		int      integerValue2,
		int      integerValue3);

	int LogMessage(
		LogLevel severityLevel,
		char*    messageFormatString,
		int      integerValue1,
		int      integerValue2,
		int      integerValue3,
		int      integerValue4);

	int LogMessage(
		LogLevel severityLevel,
		char*    messageFormatString,
		void*    pointerValue);

	int LogData(
		LogLevel severityLevel,
		char*    dataDescription,
		char*    dataBytes,
		size_t   dataByteCount);

	int LogNewLine(LogLevel eSeverityLevel);

private:
	bool     LockThisObject(void);
	bool     UnlockThisObject(void);
	void     DetermineLogFileSizeHWM(void);
	LogLevel RegularizeLogLevel(LogLevel logLevel);
	int      CheckConditionsForLog(LogLevel severityLevel);
	bool     RetrieveTempDirPath(void);
	void     FreeTempDirPath(void);
	void     FreeLogFilePath(void);
	void     FreeBackupLogFilePath(void);
	int      Open(void);
	int      WriteData(LogLevel eSeverityLevel, BYTE* pucDataBytes, size_t uiDataByteCount);
	int      FlushData(void);
	int      LogDateTime(LogLevel eSeverityLevel);
	int      LogSeverityLevel(LogLevel severityLevel);
	int      LogEventSubject(LogLevel eSeverityLevel, char* eventSubject);
	int      LogMessagePrefix(LogLevel severityLevel);
	int      LogDataPrefix(LogLevel severityLevel);
	int      LogEventPrefix(LogLevel severityLevel, char* eventSubject);

	MutexControl      oMutexControl;
	BinaryFileWriter* binaryFileWriter;
	LogLevel          effectiveLogLevel;
	LogLevel          beginLogLevelForFlushAfterWrite;
	TCHAR*            szTempDirPath;
	DWORD             dwTempDirPathSize;
	TCHAR*            szLogFilePath;
	TCHAR*            szBackupLogFilePath;
	_off_t            lLogFileSizeHWM;	// Log-file size high-water mark
	size_t            uiMaxCacheSession;
};
