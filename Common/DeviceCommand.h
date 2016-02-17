#pragma once
class DeviceCommand
{
public:
	DeviceCommand(){};
	~DeviceCommand(){ delete nextDeviceCommand; };
	int iHande;
	int iCliSocket;
	int iFuntionId;
	int dataLen;
	DeviceCommand* nextDeviceCommand;
	char commonData[1];
};