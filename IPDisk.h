#pragma once

#define CR_CLIENT				0x01000			// client config
#define CR_SERVER				0x01001			// server config

#define CR_STATUS_LISTEN		0x02001
#define CR_STATUS_RECIEVE		0x02002
#define CR_STATUS_CLOSED		0x02003
#define CR_STATUS_OPEN			0x02004

#define CR_STATUS_FAILED		0x02FFF



#define CURRENT_ADDRESS "127.0.0.1"
#define CURRENT_TCPPORT 6000

#include "diskioctl.h"

class CIPDisk
{


public:
	CIPDisk(HWND hwnd);
	virtual ~CIPDisk(void);

	BOOL m_bConnected;

	MASTER_BOOT_RECORD* mbr;


private:
	HWND	m_hwnd;			// Calling window handle
	WSADATA WSAData;		// details of the Windows Sockets implementation
	SOCKET sock;			// local socket info
	SOCKADDR_IN sa;			// socket details
	TCHAR m_czRemoteIP[32]; // remote ip address connected to us
	


public:
	int CIPDisk::InitSockets(DWORD dwLocalType, char* czIPAddr);
	BOOL IPAcceptConn(void);
	void IPSend(DWORD dwType);
	void IPRead(void);
	BOOL CheckConnectStatus(void);
	void SendMessage(LPCTSTR lpString);
};
