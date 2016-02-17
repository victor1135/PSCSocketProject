#include "DeviceCommand.h"
#pragma once
class DeviceQueue
{
public:
	DeviceQueue()
	{
		count = 0;
	}
	int size()
	{
		return count;
	}
	virtual int  EnqueueData(DeviceCommand* pDevComm) = 0;
	virtual int  DequeueData(DeviceCommand** pDevComm) = 0;
	virtual int  FrontDeviceCommand(DeviceCommand* pDevComm) = 0;

	
private:
	int count;
};

