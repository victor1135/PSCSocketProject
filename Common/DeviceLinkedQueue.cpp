
#include "stdafx.h"

#include "DeviceCommand.h"
#include "DeviceQueue.h"
#include "DeviceLinkedQueue.h"


#pragma once
DeviceLinkedQueue::DeviceLinkedQueue()
{
	count = 0;
	fristDeviceCommand = NULL;
	lastDeviceCommand = NULL;
}

DeviceLinkedQueue::~DeviceLinkedQueue()
{
	delete fristDeviceCommand;
	fristDeviceCommand = NULL;
	delete lastDeviceCommand;
	lastDeviceCommand = NULL;
}

int DeviceLinkedQueue::EnqueueData(DeviceCommand* devComm)
{
	if (count == 0)
	{
		fristDeviceCommand = devComm;
		lastDeviceCommand = devComm;
		devComm->nextDeviceCommand = NULL;
	}
	else
	{
		lastDeviceCommand->nextDeviceCommand = devComm;  //�̫�@�Ӫ��U�@��
		lastDeviceCommand = devComm;//�s�[���̫ܳ�@��
	}
	count++;
	return 0;
}

int DeviceLinkedQueue::DequeueData(DeviceCommand** devComm)
{
	if (count == 0)
	{
		return 3;
	}
	else
	{
		*devComm = fristDeviceCommand; //��̫e������devComm
		fristDeviceCommand = (*devComm)->nextDeviceCommand;//�̫e�����U�@���̫ܳe��
		(*devComm)->nextDeviceCommand = 0;
	}
	count--;
	return 0;
}

int DeviceLinkedQueue::FrontDeviceCommand(DeviceCommand* devComm)
{
	devComm = fristDeviceCommand;
	return 0;
}

int DeviceLinkedQueue::size()
{
	return count;
}