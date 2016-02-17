#include "StdAfx.h"
#include "DataLog.h"
#include "MutexControl.h"


MutexControl::MutexControl(void)
{
	mpDataLog = NULL;
	mhMutex = NULL;
	mdwMutexWaitTimeout = 1000L;
}


MutexControl::~MutexControl(void)
{
	Close();
}


void MutexControl::SetDataLogObject(DataLog* pDataLog)
{
	mpDataLog = pDataLog;
}


bool MutexControl::Create(void)
{
	bool bReturnValue = true;

	if (mhMutex == NULL)
	{
		mhMutex = CreateMutex(NULL, FALSE, NULL);

		if (mhMutex == NULL)
		{
			if (mpDataLog != NULL)
			{
				mpDataLog->LogEvent(DataLog::LogError, "CreateMutex",
					"Failed to create mutex object, system error code = 0x%08X", (int) GetLastError());
			}

			bReturnValue = false;
		}
	}

	return(bReturnValue);
}


bool MutexControl::Wait(void)
{
	static char* szFuncDesc = "WaitForMutex";
	DWORD dwWaitResult;

	if (mhMutex == NULL) Create();

	if (mhMutex == NULL)
	{
		if (mpDataLog != NULL)
			mpDataLog->LogEvent(DataLog::LogError, szFuncDesc, "Mutex object not created");

		dwWaitResult = ERROR_INVALID_HANDLE;
	}
	else
	{
		dwWaitResult = WaitForSingleObject(mhMutex, mdwMutexWaitTimeout);

		switch(dwWaitResult)
		{
			case WAIT_OBJECT_0:
				break;

			case WAIT_TIMEOUT:
				if (mpDataLog != NULL)
					mpDataLog->LogEvent(DataLog::LogError, szFuncDesc, "Timed out waiting for the mutex object to be signaled");

				break;

			case WAIT_ABANDONED:
				if (mpDataLog != NULL)
					mpDataLog->LogEvent(DataLog::LogWarning, szFuncDesc, "Mutex object abandoned");

				break;

			case WAIT_FAILED:
				if (mpDataLog != NULL)
				{
					mpDataLog->LogEvent(DataLog::LogError, szFuncDesc,
						"Failed to wait for the mutex object to be signaled, system error code = 0x%08X", (int) GetLastError());
				}

				break;

			default:
				if (mpDataLog != NULL)
					mpDataLog->LogEvent(DataLog::LogError, szFuncDesc, "Unknown mutex object wait result = 0x%08X", (int) dwWaitResult);
		}
	}

	return((dwWaitResult == WAIT_OBJECT_0) ||
		   (dwWaitResult == WAIT_ABANDONED));
}


bool MutexControl::Release(void)
{
	static char* szFuncDesc = "ReleaseMutex";
	bool bReturnValue;

	if (mhMutex == NULL)
	{
		if (mpDataLog != NULL)
			mpDataLog->LogEvent(DataLog::LogError, szFuncDesc, "Mutex object not created");

		bReturnValue = false;
	}
	else
	{
		if (ReleaseMutex(mhMutex))
			bReturnValue = true;
		else
		{
			if (mpDataLog != NULL)
			{
				mpDataLog->LogEvent(DataLog::LogError, szFuncDesc,
					"Failed to release mutex object, system error code = 0x%08X", (int) GetLastError());
			}

			bReturnValue = false;
		}
	}

	return(bReturnValue);
}


bool MutexControl::Close(void)
{
	bool bReturnValue = true;

	if (mhMutex != NULL)
	{
		if (CloseHandle(mhMutex) == FALSE)
		{
			if (mpDataLog != NULL)
			{
				mpDataLog->LogEvent(DataLog::LogError,
					"CloseMutex", "Failed to close mutex object, system error code = 0x%08X", (int) GetLastError());
			}

			bReturnValue = false;
		}

		mhMutex = NULL;
	}

	return(bReturnValue);
}
