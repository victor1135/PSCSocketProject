
#pragma once
#include "winsock2.h"
class WgSocket
{
private:
	SOCKET m_Socket;
public:
	bool Initialize();
	void Terminate();
	bool IsLocalHost(const char* hostname);
	bool GetHostIP(const char* hostname, int &ip1, int &ip2, int &ip3, int &ip4);
	WgSocket(void);
	~WgSocket();
	bool IsOpened(void) const;
	void SetSocket(SOCKET socket);
	bool Open(const char* hostname, int port);
	void Close(void);
	int  WaitInputData(SOCKET c_socket[], int seconds, int iSocketCount, bool* b_SocketClient);
	bool WaitInputData(int seconds);
	bool Read(SOCKET c_socket, void* buffer, long len, long &ret_len);
	bool Read(void* buffer, long len, long &ret_len);
	bool Write(SOCKET c_socket, const void* buffer, long len);
	bool Write(const void* buffer, long len);
	bool Listen(int port);
	bool Accept(SOCKET &socket);
	bool SetNoDelay(void);
	bool IsSocketDead(SOCKET sock);
};