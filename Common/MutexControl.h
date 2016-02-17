#pragma once

#include <Windows.h>
class DataLog;


class MutexControl
{
public:
	MutexControl(void);
	~MutexControl(void);
	void SetDataLogObject(DataLog* pDataLog);
	bool Create(void);
	bool Wait(void);
	bool Release(void);
	bool Close(void);

private:
	DataLog* mpDataLog;
	HANDLE   mhMutex;
	DWORD    mdwMutexWaitTimeout;	// in milliseconds
};
