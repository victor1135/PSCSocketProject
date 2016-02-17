#include "DeviceCommand.h"
#include "DeviceQueue.h"

#pragma once
class DeviceLinkedQueue : DeviceQueue
{
public:
	DeviceLinkedQueue();
	~DeviceLinkedQueue();
	int EnqueueData(DeviceCommand* pDevComm);
	int DequeueData(DeviceCommand**  pDevComm);
	int FrontDeviceCommand(DeviceCommand* pDevComm);
	int size();

private:
	DeviceCommand* fristDeviceCommand;
	DeviceCommand* lastDeviceCommand;
	int count;
};

