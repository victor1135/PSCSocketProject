#include "StdAfx.h"
#include <errno.h>
#include "BinaryFileWriter.h"
#include "GetTempDirPath.h"
#include "ChangeFileNameExt.h"
#include "FileExists.h"
#include "GetFileSize.h"
#include "DateTimeHelpers.h"
#include "DataDump.h"
#include "DataLog.h"
#include <strsafe.h>



DataLog::DataLog(void)
{
	binaryFileWriter = NULL;
	effectiveLogLevel = LogWarning;
	beginLogLevelForFlushAfterWrite = LogSummary;
	szTempDirPath = NULL;
	szLogFilePath = NULL;
	szBackupLogFilePath = NULL;
	uiMaxCacheSession = 0;
	oMutexControl.SetDataLogObject(this);
	DetermineLogFileSizeHWM();
}


DataLog::~DataLog(void)
{
	Close();
	FreeTempDirPath();
	FreeLogFilePath();
	FreeBackupLogFilePath();
	oMutexControl.SetDataLogObject(NULL);
	oMutexControl.Close();
}


bool DataLog::LockThisObject(void)
{
	return(oMutexControl.Wait());
}


bool DataLog::UnlockThisObject(void)
{
	return(oMutexControl.Release());
}


void DataLog::DetermineLogFileSizeHWM(void)
{
	lLogFileSizeHWM = 1;

	if (effectiveLogLevel >= LogWarning)
		lLogFileSizeHWM <<= (effectiveLogLevel - 1);

	lLogFileSizeHWM *= (1024 * 1024);
}


DataLog::LogLevel DataLog::RegularizeLogLevel(LogLevel logLevel)
{
	if (logLevel < LogNone)
		logLevel = LogNone;
	else
	{
		if (logLevel > LogVeryVerbose)
			logLevel = LogVeryVerbose;
	}

	return(logLevel);
}


void DataLog::SetLogLevel(LogLevel logLevel)
{
	LogLevel eNewLogLevel;
	bool     bObjectLocked;

	eNewLogLevel = RegularizeLogLevel(logLevel);
	bObjectLocked = LockThisObject();

	if (effectiveLogLevel != eNewLogLevel)
	{
		effectiveLogLevel = eNewLogLevel;
		DetermineLogFileSizeHWM();

		LogEvent(LogForced, "SetLogLevel", "New data-log level = %d, data-log file size HWM = %u Mbytes",
			(int) effectiveLogLevel, (int) (lLogFileSizeHWM / (1024 * 1024)));

		Close();
	}

	if (bObjectLocked) UnlockThisObject();
}


// Returns true if the given log level is effective, otherwise, returns false.
bool DataLog::IsEffectiveLogLevel(LogLevel logLevel)
{
	bool bObjectLocked;
	bool bReturnValue;

	bObjectLocked = LockThisObject();
	bReturnValue = (logLevel <= effectiveLogLevel);
	if (bObjectLocked) UnlockThisObject();
	return(bReturnValue);
}


int DataLog::CheckConditionsForLog(LogLevel severityLevel)
{
	int iReturnValue;

	if ((severityLevel <= effectiveLogLevel) ||
		(uiMaxCacheSession > 0))
		iReturnValue = 0;
	else
		iReturnValue = EACCES;

	return(iReturnValue);
}


// Returns true if successful; false on failure.
bool DataLog::RetrieveTempDirPath(void)
{
	bool bRetValue;

	if (szTempDirPath != NULL)
		bRetValue = true;
	else
	{
		if (GetTempDirPath(&szTempDirPath, &dwTempDirPathSize) == ERROR_SUCCESS)
			bRetValue = true;
		else
		{
			szTempDirPath = NULL;
			dwTempDirPathSize = 0;
			bRetValue = false;
		}
	}

	return(bRetValue);
}


void DataLog::FreeTempDirPath(void)
{
	if (szTempDirPath != NULL)
	{
		free((void*) szTempDirPath);
		szTempDirPath = NULL;
		dwTempDirPathSize = 0;
	}
}


// Returns true if successful; false on failure.
bool DataLog::SetLogFilePath(TCHAR* szLogFilePath)
{
	size_t uiPathBufSize;
	TCHAR* pcPathBuf;
	bool   bObjectLocked;
	bool   bRetValue = false;

	uiPathBufSize = (_tcslen(szLogFilePath) + 1) * sizeof(TCHAR);

	if (uiPathBufSize > sizeof(TCHAR))
	{
		pcPathBuf = (TCHAR*) malloc(uiPathBufSize);

		if (pcPathBuf != NULL)
		{
			memcpy(pcPathBuf, szLogFilePath, uiPathBufSize);
			bObjectLocked = LockThisObject();
			FreeLogFilePath();
			this->szLogFilePath = pcPathBuf;
			FreeBackupLogFilePath();
			szBackupLogFilePath = ChangeFileNameExtension(pcPathBuf, TEXT(".bak"));
			if (bObjectLocked) UnlockThisObject();
			bRetValue = true;
		}
	}

	return(bRetValue);
}


// Returns true if successful; false on failure.
bool DataLog::SetLogFileNameInTemp(TCHAR* szLogFileName)
{
	bool  bObjectLocked;
	TCHAR acLogFilePath[_MAX_PATH];
	bool  bRetValue = false;

	bObjectLocked = LockThisObject();

	if (RetrieveTempDirPath())
	{
		if (SUCCEEDED(StringCchPrintf(
			acLogFilePath,
			sizeof(acLogFilePath) / sizeof(TCHAR),
			TEXT("%s%s"),
			szTempDirPath,
			szLogFileName)))
		{
			if (SetLogFilePath(acLogFilePath))
			{
				//LogEvent(LogForced, "TEST", "szTempDirPath = \"%s\"", szTempDirPath);
				//LogEvent(LogForced, "TEST", "szLogFilePath = \"%s\"", szLogFilePath);
				//LogEvent(LogForced, "TEST", "szBackupLogFilePath = \"%s\"", szBackupLogFilePath);
				//LogEvent(LogForced, "TEST", "Log file size HWM = %u Mbytes", (int) (lLogFileSizeHWM / (1024 * 1024)));
				//Close();
				bRetValue = true;
			}
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(bRetValue);
}


void DataLog::FreeLogFilePath(void)
{
	if (szLogFilePath != NULL)
	{
		free((void*) szLogFilePath);
		szLogFilePath = NULL;
	}
}


void DataLog::FreeBackupLogFilePath(void)
{
	if (szBackupLogFilePath != NULL)
	{
		free((void*) szBackupLogFilePath);
		szBackupLogFilePath = NULL;
	}
}


// Returns 0 if successful; an error code on failure.
int DataLog::Open(void)
{
	_off_t lFileSize;
	bool   bOkToRenameFile;
	int    iReturnValue;

	do
	{
		if (binaryFileWriter != NULL)
		{
			iReturnValue = 0;
			break;
		}

		if (szLogFilePath == NULL)
		{
			iReturnValue = EINVAL;
			break;
		}

		if (FileExists(szLogFilePath))
		{
			if (GetFileSize(szLogFilePath, &lFileSize) != 0)
				DeleteFile(szLogFilePath);
			else
			{
				if (lFileSize >= lLogFileSizeHWM)
				{
					if (szBackupLogFilePath != NULL)
					{
						bOkToRenameFile = true;

						if (FileExists(szBackupLogFilePath))
						{
							if (DeleteFile(szBackupLogFilePath) == FALSE)
								bOkToRenameFile = false;
						}

						if (bOkToRenameFile)
							MoveFile(szLogFilePath, szBackupLogFilePath);
					}

					if (FileExists(szLogFilePath))
						DeleteFile(szLogFilePath);
				}
			}
		}

		binaryFileWriter = new BinaryFileWriter;
		iReturnValue = binaryFileWriter->Open(szLogFilePath);

		if (iReturnValue != 0)
		{
			delete binaryFileWriter;
			binaryFileWriter = NULL;
		}
	}
	while(false);

	return(iReturnValue);
}


// Returns 0 if the log file is successfully closed.
// Returns EOF to indicate an error.
int DataLog::Close(void)
{
	bool bObjectLocked;
	int  iReturnValue;

	bObjectLocked = LockThisObject();

	if (binaryFileWriter == NULL)
		iReturnValue = 0;
	else
	{
		iReturnValue = binaryFileWriter->Close();
		delete binaryFileWriter;
		binaryFileWriter = NULL;
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Writes data to the log file.
// Returns 0 if successful; an error code on failure.
int DataLog::WriteData(
	LogLevel eSeverityLevel,
	BYTE*    pucDataBytes,		// Pointer to the data bytes to be written.
	size_t   uiDataByteCount)	// Number of data bytes to be written.
{
	int iReturnValue;

	do
	{
		if ((pucDataBytes == NULL) || (uiDataByteCount <= 0))
		{
			iReturnValue = EINVAL;
			break;
		}

		if (uiMaxCacheSession > 0)
		{
			// Cache operations.
		}

		if (eSeverityLevel > effectiveLogLevel)
		{
			iReturnValue = 0;
			break;
		}

		if ((iReturnValue = Open()) != 0)
			break;

		iReturnValue = binaryFileWriter->Write(pucDataBytes, uiDataByteCount);
	}
	while(false);

	return(iReturnValue);
}


// Flushes data to the log file.
// Returns 0 if successful; an error code on failure.
int DataLog::FlushData(void)
{
	int iReturnValue;

	do
	{
		if (binaryFileWriter == NULL)
		{
			iReturnValue = ENOTTY;
			break;
		}

		iReturnValue = binaryFileWriter->Flush();
	}
	while(false);

	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogDateTime(LogLevel eSeverityLevel)
{
	char   dateTimeBuffer[sizeof(CURRENT_LOCAL_DATE_TIME_CHAR_STRING_FORMAT) + 10];
	size_t dateTimeLength;
	int    iReturnValue;

	dateTimeLength = GetCurrentLocalDateTimeCharString(dateTimeBuffer, sizeof(dateTimeBuffer));

	if (dateTimeLength <= 0)
		iReturnValue = ENOMEM;
	else
	{
		dateTimeBuffer[dateTimeLength] = 0x20;
		iReturnValue = WriteData(eSeverityLevel, (BYTE*) dateTimeBuffer, dateTimeLength + 1);
	}

	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogSeverityLevel(LogLevel severityLevel)
{
	static char* severityLevelText[] =
	{
		"  FORCED   ",
		"   ERROR   ",
		"  WARNING  ",
		"   INFO    ",
		"  SUMMARY  ",
		"  DETAILS  ",
		"  VERBOSE  ",
		"SUPERFLUOUS"
	};

	char    writeBuffer[32];
	size_t  writeLength;
	HRESULT resultValue;
	int     iReturnValue;

	severityLevel = RegularizeLogLevel(severityLevel);

	resultValue = StringCchPrintfA(
		(STRSAFE_LPSTR) writeBuffer,
		sizeof(writeBuffer),
		"[%s] ",
		severityLevelText[((size_t) severityLevel) - ((size_t) LogForced)]);

	if (FAILED(resultValue))
		iReturnValue = ENOMEM;
	else
	{
		writeLength = strlen(writeBuffer);
		iReturnValue = WriteData(severityLevel, (BYTE*) writeBuffer, writeLength);
	}

	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEventSubject(
	LogLevel eSeverityLevel,
	char*    eventSubject)
{
	char    formatString[] = "%08X-%08X %s : ";
	DWORD   dwCurrentProcessId;
	DWORD   dwCurrentThreadId;
	char*   writeBuffer;
	size_t  writeBufferLen;
	size_t  writeLength;
	HRESULT resultValue;
	int     iReturnValue;

	writeBufferLen = (sizeof(dwCurrentProcessId) * 2) + (sizeof(dwCurrentThreadId) * 2) + strlen(eventSubject) + sizeof(formatString);
	writeBuffer = (char*) malloc(writeBufferLen);

	if (writeBuffer == NULL)
		iReturnValue = ENOMEM;
	else
	{
		dwCurrentProcessId = GetCurrentProcessId();
		dwCurrentThreadId = GetCurrentThreadId();

		resultValue = StringCchPrintfA(
			(STRSAFE_LPSTR) writeBuffer,
			writeBufferLen,
			formatString,
			dwCurrentProcessId,
			dwCurrentThreadId,
			eventSubject);

		if (FAILED(resultValue))
			iReturnValue = ENOMEM;
		else
		{
			writeLength = strlen(writeBuffer);
			iReturnValue = WriteData(eSeverityLevel, (BYTE*) writeBuffer, writeLength);
		}

		free((void*) writeBuffer);
	}

	return(iReturnValue);
}


int DataLog::LogEventPrefix(//標頭資料
	LogLevel severityLevel,
	char*    eventSubject)
{
	int iReturnValue;

	iReturnValue = CheckConditionsForLog(severityLevel);

	if (iReturnValue == 0)
	{
		iReturnValue = LogDateTime(severityLevel);//時間

		if (iReturnValue == 0)
		{
			iReturnValue = LogSeverityLevel(severityLevel);//等級

			if (iReturnValue == 0)
				iReturnValue = LogEventSubject(severityLevel, eventSubject);//ID
		}
	}

	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    message)
{
	bool   bObjectLocked;
	size_t messageLength;
	int    iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageLength = strlen(message);
		iReturnValue = WriteData(severityLevel, (BYTE*) message, messageLength);

		if (iReturnValue == 0)
			iReturnValue = LogNewLine(severityLevel);
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	int      integerValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + (11 - 2) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	int      integerValue1,
	int      integerValue2)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + ((11 - 2) * 2) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue1,
				integerValue2);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	int      integerValue1,
	int      integerValue2,
	int      integerValue3)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + ((11 - 2) * 3) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue1,
				integerValue2,
				integerValue3);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	int      integerValue,
	double   doubleValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + 32;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue,
				doubleValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	void*    pointerValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + 256;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				pointerValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogEvent(
	LogLevel severityLevel,
	char*    eventSubject,
	char*    messageFormatString,
	void*    pointerValue,
	int      integerValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogEventPrefix(severityLevel, eventSubject);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + 256 + 16;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				pointerValue,
				integerValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


int DataLog::LogMessagePrefix(LogLevel severityLevel)
{
	int iReturnValue;

	iReturnValue = CheckConditionsForLog(severityLevel);
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    message)
{
	bool   bObjectLocked;
	size_t messageLength;
	int    iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageLength = strlen(message);
		iReturnValue = WriteData(severityLevel, (BYTE*) message, messageLength);

		if (iReturnValue == 0)
			iReturnValue = LogNewLine(severityLevel);
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    messageFormatString,
	int      integerValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + (11 - 2) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    messageFormatString,
	int      integerValue1,
	int      integerValue2)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + ((11 - 2) * 2) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue1,
				integerValue2);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    messageFormatString,
	int      integerValue1,
	int      integerValue2,
	int      integerValue3)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + ((11 - 2) * 3) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue1,
				integerValue2,
				integerValue3);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    messageFormatString,
	int      integerValue1,
	int      integerValue2,
	int      integerValue3,
	int      integerValue4)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + ((11 - 2) * 4) + 1;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				integerValue1,
				integerValue2,
				integerValue3,
				integerValue4);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogMessage(
	LogLevel severityLevel,
	char*    messageFormatString,
	void*    pointerValue)
{
	bool    bObjectLocked;
	char*   messageBuffer;
	size_t  messageBufferLen;
	size_t  messageLength;
	HRESULT resultValue;
	int     iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogMessagePrefix(severityLevel);

	if (iReturnValue == 0)
	{
		messageBufferLen = strlen(messageFormatString) + 256;
		messageBuffer = (char*) malloc(messageBufferLen);

		if (messageBuffer == NULL)
			iReturnValue = ENOMEM;
		else
		{
			resultValue = StringCchPrintfA(
				(STRSAFE_LPSTR) messageBuffer,
				messageBufferLen,
				messageFormatString,
				pointerValue);

			if (FAILED(resultValue))
				iReturnValue = ENOMEM;
			else
			{
				messageLength = strlen(messageBuffer);
				iReturnValue = WriteData(severityLevel, (BYTE*) messageBuffer, messageLength);

				if (iReturnValue == 0)
					iReturnValue = LogNewLine(severityLevel);
			}

			free((void*) messageBuffer);
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


int DataLog::LogDataPrefix(LogLevel severityLevel)
{
	int iReturnValue;

	iReturnValue = CheckConditionsForLog(severityLevel);
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogData(
	LogLevel severityLevel,
	char*    dataDescription,
	char*    dataBytes,
	size_t   dataByteCount)
{
	bool   bObjectLocked;
	char*  dataDumpOutput;
	size_t dataDumpOutputLen;
	int    iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = LogDataPrefix(severityLevel);

	if (iReturnValue == 0)
	{
		dataDumpOutputLen = DumpData(dataDescription, dataBytes, dataByteCount, &dataDumpOutput);

		if (dataDumpOutputLen <= 0)
			iReturnValue = ENOMEM;
		else
		{
			iReturnValue = WriteData(severityLevel, (BYTE*) dataDumpOutput, dataDumpOutputLen);
			free((void*) dataDumpOutput);
			if (effectiveLogLevel >= beginLogLevelForFlushAfterWrite) FlushData();
		}
	}

	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}


// Returns a non-zero error code on failure.
int DataLog::LogNewLine(LogLevel eSeverityLevel)
{
	bool bObjectLocked;
	int  iReturnValue;

	bObjectLocked = LockThisObject();
	iReturnValue = WriteData(eSeverityLevel, (BYTE*) "\r\n", 2);
	if (effectiveLogLevel >= beginLogLevelForFlushAfterWrite) FlushData();
	if (bObjectLocked) UnlockThisObject();
	return(iReturnValue);
}
