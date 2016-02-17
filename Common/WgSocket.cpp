
#include "WgSocket.h"
#include "winsock2.h"
#include <iostream>
#include "SocketDeviceErrorCodes.h"

static bool b_Init_Flag = false;

bool WgSocket::Initialize()
{
#ifdef WIN32
	if (!b_Init_Flag)
	{
		WSAData wsa_data;
		if (WSAStartup(0x202, &wsa_data) != 0) return false;
		b_Init_Flag = true;
	}
#endif
	return true;
}

void WgSocket::Terminate()
{
#ifdef WIN32
	if (b_Init_Flag)
	{
		WSACleanup();
		b_Init_Flag = false;
	}
#endif
}

WgSocket::WgSocket(void)
//�����G�غcSocket����  
{
	m_Socket = INVALID_SOCKET;
}

WgSocket :: ~WgSocket()
//�����G�ѺcSocket����
{
	Close();
}

bool WgSocket::IsLocalHost(const char* hostname)
//�����G�ˬd�O�_��localhost�I�s  l
//��J�Ghostname = Server��}  
//�Ǧ^�G�O�_��localhost�I�s  
{
	if (hostname == NULL) return true;
	if (*hostname == 0) return true;
	if (lstrcmpA(hostname, "localhost") == 0) return true;
	if (lstrcmpA(hostname, "127.0.0.1") == 0) return true;
	return false;
}
bool WgSocket::IsOpened(void) const
//�����G�˴�Socket�O�_�w�}��  
//�Ǧ^�G�˴����G  
{
	if (m_Socket == INVALID_SOCKET) return false;
	return true;
}
/*
* WSAECONNREFUSED (10061): �L�k�s�u
* WSAEADDRINUSE (10048): �S���i�Ϊ�port
* WSAETIMEDOUT (10060): �s�u�O��
*/
bool WgSocket::Open(const char* hostname, int port)
//�����G�}�һPServer���s�u  
//��J�Ghostname,port = Server��}�P�q�T��  
//�Ǧ^�G���ѶǦ^false 
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	// �ѥXsocket address //
	if (IsLocalHost(hostname)) hostname = "127.0.0.1";
	sock_addr.sin_family = AF_INET;//�Y�ϥ�IP
	sock_addr.sin_port = htons(port);
	struct hostent* hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) return false;
	sock_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	// �إ�socket //  
	try
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	}
	catch (...)
	{
		m_Socket = INVALID_SOCKET;
		return false;
	}
	if (m_Socket == INVALID_SOCKET) return false;
	// �}�l�s�u //  
	try
	{
		if (connect(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) >= 0) return true;
	}
	catch (...)
	{
		Close();
		return false;
	}
	// ���B�i�H�[�J�@�ǿ��~�B�z... //  
}

void WgSocket::Close(void)
//�����G�����PServer���s�u  
{
	if (!IsOpened()) return;
	try
	{
#ifdef WIN32  
		// http://msdn.microsoft.com/en-us/library/ms740481(v=vs.85).aspx  
		shutdown(m_Socket, SD_SEND);
#else  
		// http://linux.die.net/man/3/shutdown  
		shutdown(m_Socket, SHUT_WR);
#endif 
	}
	catch (...)
	{
		closesocket(m_Socket);
	}
	m_Socket = INVALID_SOCKET;
}
/*
* - Server�ݪ���ť�L�{
*      Server�ݪ���ť�L�{, �D�n�Obind, listen (�H�W�u�ݤ@��), �M��Q��accept�ӻPClient
*      �ݫإ߳s�u (�H�W�i�H�h��).
*/
bool WgSocket::Listen(int port)
//�����G��ť�Y��Port  
//��J�Gport = ��ťPort  
//�Ǧ^�G���ѶǦ^false 
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(port);

	// �إ�socket // 
	try
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	}
	catch (...)
	{
		m_Socket = INVALID_SOCKET;
		return false;
	}
	if (m_Socket == INVALID_SOCKET) return false;
	// Bind socket //
	int on = 1;
	// http://msdn.microsoft.com/en-us/library/ms740476(v=vs.85).aspx  
	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	int rc;
	try
	{
		// http://msdn.microsoft.com/en-us/library/ms737550(v=vs.85).aspx  
		rc = bind(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));

	}
	catch (...)
	{
		//printf("%d", WSAGetLastError());

		rc = SOCKET_ERROR;
	}
	if (rc == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	// Listen socket //  
	try
	{
		// http://msdn.microsoft.com/en-us/library/ms739168(v=vs.85).aspx  
		rc = listen(m_Socket, SOMAXCONN);
	}
	catch (...)
	{
		rc = SOCKET_ERROR;
	}
	if (rc == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	return true;
}

bool WgSocket::Accept(SOCKET &socket)
//�����G���ݱ����s�u  
//��X�G�s�usocket  
//�Ǧ^�G���ѶǦ^false  
{
	socket = INVALID_SOCKET;
	if (!IsOpened()) return false;
	struct sockaddr_in from;
#ifdef WIN32  
	int fromlen = sizeof(from);
	//	printf("%d", WSAGetLastError());
#else  
	socklen_t fromlen = sizeof(from);
#endif  
	try
	{
		socket = accept(m_Socket, (struct sockaddr*)&from, &fromlen);
		//printf("%d", WSAGetLastError());
	}
	catch (...)
	{
		socket = INVALID_SOCKET;
		return false;
	}
	//	printf("%d", WSAGetLastError());
	return true;
}

/*
* �ѩ�read��ƥ��������観�e�Ӹ�Ʈɤ~�|��^, �]���b�I�sread�e�̦n�������O�_����ƶi��,
* �H�K�i��timeout�B�z. �o�̧ڥu�H"��"��timeout�����, �ݭn��ӷL���ɶ�, �Цۦ�ק�.
*/
bool WgSocket::WaitInputData(int seconds)
//�����G���ݹ��e�Ӹ��  
//��J�Gseconds = ���ݬ��  
//�Ǧ^�G�S����ƶǦ^false  
{
	if (!IsOpened())
	{
		//printf("\t[WaitInputData] Socket Not Open Yet!\n");
		return false;
	}
	// �]�wdescriptor sets //  
	fd_set socket_set;
	FD_ZERO(&socket_set);
	FD_SET((unsigned int)m_Socket, &socket_set);
	// �]�wtimeout�ɶ� //  
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	// �����O�_����� //  
	try
	{
		if (select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout) <= 0)
		{
			printf("\t[WaitInputData] Timeout!\n");
			return false;
		}
	}
	catch (...)
	{
		//printf("\t[WaitInputData] Exception!\n");
		return false;
	}
	return true;
}

int WgSocket::WaitInputData(SOCKET c_socket[], int seconds,int iSocketCount,bool* b_SocketClient)
//�����G���ݹ��e�Ӹ��  
//��J�Gseconds = ���ݬ��  
//�Ǧ^�G�S����ƶǦ^false  
{
	int iReturnCode = 0;
	if (!IsOpened())
	{
		//printf("\t[WaitInputData] Socket Not Open Yet!\n");
		return false;
	}
	// �]�wdescriptor sets //  
	fd_set socket_set;
	FD_ZERO(&socket_set);
	for (int i = 0; i < iSocketCount; i++)
	{
		FD_SET(c_socket[i], &socket_set);
	}

	// �]�wtimeout�ɶ� //  
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	// �����O�_����� //  
	try
	{
		iReturnCode = select(0, &socket_set, NULL, NULL, &timeout);
		if (iReturnCode  == 0)
		{
			iReturnCode = SOCKETDEVICE_OPERATION_TIMED_OUT;
			printf("\t[WaitInputData] Timeout!\n");
			return iReturnCode;
		}
		for (int i = 0; i < 10; i++)
		{
			if (FD_ISSET(c_socket[i], &socket_set))
			{
				b_SocketClient[i] = true;
			}
		}
	}
	catch (...)
	{
		//printf("\t[WaitInputData] Exception!\n");
		return false;
	}
	return 0;
}

bool WgSocket::Read(void* data, long len, long &ret_len)
//�����GŪ�����  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//��X�Gdata = Ū�������, ret_len = ���Ū������Ƥj�p�A0����w�_�u  
//�Ǧ^�G���ѶǦ^false  
//�Ƶ��G����Ʒ|�@�����즳Ū����Ʃε����s�u�ɤ~�Ǧ^  
{
	ret_len = 0;
	if (!IsOpened()) return true;
	try
	{
#ifndef WIN32  
		signal(SIGPIPE, SIG_IGN); // �קKSIGPIPE�T���פ�{�� //  
#endif  
		// http://msdn.microsoft.com/en-us/library/ms740121(v=vs.85).aspx  
		ret_len = recv(m_Socket, (char*)data, len, 0);
	}
	catch (...)
	{
		ret_len = SOCKET_ERROR;
	}
	if (ret_len < 0)
	{
		ret_len = 0;
		return false;
	}
	return true;
}

bool WgSocket::Read(SOCKET c_socket, void* data, long len, long &ret_len)
//�����GŪ�����  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//��X�Gdata = Ū�������, ret_len = ���Ū������Ƥj�p�A0����w�_�u  
//�Ǧ^�G���ѶǦ^false  

//�Ƶ��G����Ʒ|�@�����즳Ū����Ʃε����s�u�ɤ~�Ǧ^  
{
	ret_len = 0;
	if (!IsOpened()) return true;
	try
	{
#ifndef WIN32  
		signal(SIGPIPE, SIG_IGN); // �קKSIGPIPE�T���פ�{�� //  
#endif  
		// http://msdn.microsoft.com/en-us/library/ms740121(v=vs.85).aspx  
		ret_len = recv(c_socket, (char*)data, len, 0);
	}
	catch (...)
	{
		ret_len = SOCKET_ERROR;
	}
	if (ret_len < 0)
	{
		ret_len = 0;
		return false;
	}
	return true;
}

bool WgSocket::Write(const void* data, long len)
//�����G�e�X���  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//�Ǧ^�G���ѶǦ^false  
{
	if (!IsOpened()) return false;
	if (len <= 0) return true;
	int write_len;
	try
	{
#ifndef WIN32  
		signal(SIGPIPE, SIG_IGN); // �קKSIGPIPE�T���פ�{�� //  
#endif  
		write_len = send(m_Socket, (const char*)data, len, 0);
	}
	catch (...)
	{
		write_len = SOCKET_ERROR;
	}

	if (write_len != len) return false;
	return true;
}


bool WgSocket::Write(SOCKET c_socket, const void* data, long len)
//�����G�e�X���  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//�Ǧ^�G���ѶǦ^false  
{
	if (!IsOpened()) return false;
	if (len <= 0) return true;
	int write_len;
	try
	{
#ifndef WIN32  
		signal(SIGPIPE, SIG_IGN); // �קKSIGPIPE�T���פ�{�� //  
#endif  
		write_len = send(c_socket, (const char*)data, len, 0);
	}
	catch (...)
	{
		write_len = SOCKET_ERROR;
	}
	
	if (write_len != len) return false;
	return true;
}

bool WgSocket::GetHostIP(const char* hostname, int &ip1, int &ip2, int &ip3, int &ip4)
//�����G���o���whost��ip  
//��J�Ghostname = host��}  
//��X�Gip1-4 = ip��}  
//�Ǧ^�G���ѶǦ^false  
{
	if (IsLocalHost(hostname))
	{
		// �����X��ڪ�hostname //  
		struct hostent *hostinfo = gethostbyname("localhost");
		if (hostinfo == NULL) return false;
		hostname = hostinfo->h_name;
	}
	struct hostent* hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) return false;
	char* addr = hostinfo->h_addr_list[0];
	ip1 = (unsigned char)addr[0];
	ip2 = (unsigned char)addr[1];
	ip3 = (unsigned char)addr[2];
	ip4 = (unsigned char)addr[3];
	return true;
}

/*
* Nagle Algorithm���Բӻ���, �аѦ�MSDN "Nagle Algorithm"�@��, �o�Ӻt��k�D�n�O�קK�L
* �h�s�����e�X���, �N��������A�@���e�X. ���D���s�Ĳv�B�D�@���ʸ�ưe�X���q�T�{��
* �Ө� (�ҦpTTY, telnet��), �o�Ӻt��k�i�H�j�q���C��������ƶǿ�q. ���Y�O�w�]�p�n�@��
* �ʫʥ]���q�T�n��Ө�, �o�Ӻt��k�Ϧӷ|�Y���v�T�Ĳv.
* �Ӱ���Nagle Algorithm, �o�]�O����ƪ��D�n�ت�.
* ��h�i�H�Ѧ� :
* http://www-128.ibm.com/developerworks/linux/library/l-hisock.html?ca=dgr-lnxw01BoostSocket
*/
bool WgSocket::SetNoDelay(void)
//�����G�]�w������ǰe (����Nagle Algorithm)  
//�Ǧ^�G�]�w���ѶǦ^false  
{
	if (!IsOpened()) return false;
	int on = 1;
	if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)) != 0) return false;
	return true;
}


bool WgSocket::IsSocketDead(SOCKET sock)
{

	char buf;
	int err = recv(sock, &buf, 1, MSG_PEEK);
	if (err == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			return false;
		}
	}
	return true;
}