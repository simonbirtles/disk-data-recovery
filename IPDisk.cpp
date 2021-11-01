#include "StdAfx.h"
#include "IPDisk.h"
// Af_irda.h


#include "diskanalysis.h"
#include "diskioctl.h"

#define DD_ERROR		0x11FF
#define DD_TEXT			0x1100
#define DD_MBR			0x1101
#define DD_BOOT			0x1102
#define DD_FINDFILE		0x1103
#define DD_RECOVERFILE	0x1104
#define DD_MFTRECORD	0x1105

#define WMS_SOCKET_NOTIFY		WM_USER+1
#define WMS_ADD_REMOTE			WM_USER+2

CIPDisk::CIPDisk(HWND hwnd)
{
	
	m_hwnd = hwnd;	// save for later
	m_bConnected = false;
	mbr = NULL;

}

CIPDisk::~CIPDisk(void)
{
}



int CIPDisk::InitSockets(DWORD dwLocalType, char* czIPAddr)
{
	int iRet = 0;
	
	
    if(iRet = WSAStartup(MAKEWORD(2,0), &WSAData))
	{
		TRACE1("\nWinSock startup error : #%i", iRet);
		return CR_STATUS_FAILED;
	}

	switch(dwLocalType)
	{
	case CR_CLIENT:
		               // (Internet, TCP, IP)  winsock2.h
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		if(sock == INVALID_SOCKET)
		{
			TRACE0("\nFailed to create socket");
			WSACleanup();
			return CR_STATUS_FAILED;
		}

		if(SOCKET_ERROR == WSAAsyncSelect(sock, m_hwnd, WMS_SOCKET_NOTIFY , FD_ACCEPT|FD_CONNECT|FD_READ|FD_WRITE))
			return CR_STATUS_FAILED;

						  // setup connection
		sa.sin_family = AF_INET;
		sa.sin_addr.S_un.S_addr = inet_addr(czIPAddr);
		sa.sin_port = htons(CURRENT_TCPPORT);

		connect(sock, (struct sockaddr FAR *)&sa, sizeof(sa) );
		if(WSAEWOULDBLOCK != (iRet = WSAGetLastError()) )
		{
			TRACE1("\nWinSock bind error on client: #%i", iRet);
			return CR_STATUS_FAILED;
		}

	    return CR_STATUS_LISTEN;
		break;

	case CR_SERVER:	   
					   // (Internet, TCP, IP)  winsock2.h
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if(sock == INVALID_SOCKET)
		{
			TRACE0("\nFailed to create socket");
			WSACleanup();
			return CR_STATUS_FAILED;
		}
					  // startup listening 
		sa.sin_addr.S_un.S_addr = inet_addr(CURRENT_ADDRESS);
		sa.sin_family = AF_INET;
		sa.sin_port = htons(CURRENT_TCPPORT);
        

		if(SOCKET_ERROR == WSAAsyncSelect(sock, m_hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT|FD_CONNECT|FD_READ|FD_WRITE))
			return CR_STATUS_FAILED;


					  // bind the above port/ip address to the socket
		bind(sock, (SOCKADDR*)&sa, sizeof(sa) );
		if(WSAEWOULDBLOCK != (iRet = WSAGetLastError()) )
		{
			//TRACE1("\nWinSock bind error : #%i", iRet);
			//return CR_STATUS_FAILED;
		}
		WSASetLastError(0);

					  // start listening on this socket
		listen( sock, SOMAXCONN);
	//	connect( sock, (SOCKADDR*)&sa, sizeof(sa));

		if(WSAEWOULDBLOCK != (iRet = WSAGetLastError()) )
		{
			//TRACE1("\nWinSock listen startup error : #%i", iRet);
			//return CR_STATUS_FAILED;
		}
					 // return happy 
		return CR_STATUS_LISTEN;
		break;

	default:
		// TODO
		break;
	}

	return CR_STATUS_FAILED;
}


/*
  we dont accept any connections !!
*/
BOOL CIPDisk::IPAcceptConn(void)
{	/*
	sockaddr sa;
	int *pi = 0;
	SOCKET sock2;

	if(INVALID_SOCKET == (sock2 = accept(sock, &sa, pi)))
	{
		DWORD dwErr = WSAGetLastError();
		TRACE1("\nIncoming connection attempt failed : #%i", dwErr);
		m_bConnected = false;
	}
	else
	{
		sock = sock2;
		m_bConnected = true;
	}


	return m_bConnected;
	*/
	return false;
}

void CIPDisk::IPSend(DWORD dwType)
{

	char* lpData = (char*)VirtualAlloc(NULL, 8, MEM_COMMIT, PAGE_READWRITE);	   //1080

	// copy data size to first 4 byte
	lpData[0] = 0;
	lpData[1] = 0;
	lpData[2] = 0;
	lpData[3] = 0;

	// copy data type  TODO
	lpData[4] = (dwType & 255);
	lpData[5] = (dwType & 65535) >> 8;
	lpData[6] = dwType >> 16;
	lpData[7] = dwType >> 24;
    
	// send data	
	if(SOCKET_ERROR == send(sock, lpData, 8, 0))
	{
		DWORD dwErr = WSAGetLastError();
		TRACE1("\nFailed to send data error #%i", dwErr);
	}


	return;

}

/*
  recieved data packet, find out what it is and decipher
*/
void CIPDisk::IPRead(void)
{
	char *pStr;
	pStr = (char*)VirtualAlloc(NULL, 1080, MEM_COMMIT, PAGE_READWRITE);

	int iBytes = recv(sock, pStr, 8, MSG_PEEK);		// look at the first 4 bytes to get data info

	__int32 iSize = pStr[0] + (pStr[1] << 8) + (pStr[2] << 16) + (pStr[3] << 24);		// data size
	DWORD  dwType = pStr[4] + (pStr[5] << 8) + (pStr[6] << 16) + (pStr[7] << 24);		// data type
   


   switch(dwType)
   {
   case DD_ERROR:
	   break;

   case DD_MBR:						// master boot record details
//	   MASTER_BOOT_RECORD* mbr;
	   if(mbr != NULL)
	   {
		   VirtualFree(mbr, 0, MEM_RELEASE);
		   mbr = NULL;
	   }
	   mbr = (MASTER_BOOT_RECORD*)VirtualAlloc(NULL, iSize, MEM_COMMIT, PAGE_READWRITE);
	   VirtualFree(pStr, 0, MEM_RELEASE);
	   pStr = (char*)VirtualAlloc(NULL, 8+iSize, MEM_COMMIT, PAGE_READWRITE);
	   recv(sock, pStr, 8+iSize, 0);    
	   pStr += 8;
	   memcpy((char*)mbr, pStr, iSize);
	   ::SendMessage(m_hwnd, WMS_ADD_REMOTE, (WPARAM)mbr, (LPARAM)(iSize/(sizeof(MASTER_BOOT_RECORD))));
	   TRACE0("");
	   break;
   
	case DD_BOOT:
		break;
	case DD_FINDFILE:
		break;
	case DD_RECOVERFILE:
		break;
	case DD_MFTRECORD:
		break;
   default:
	   break;

   }


	
	VirtualFree(pStr, 0, MEM_RELEASE);
	TRACE0("");
	
	return;
}

BOOL CIPDisk::CheckConnectStatus(void)
{
	 return m_bConnected;
}

void CIPDisk::SendMessage(LPCTSTR lpString)
{

	int iSize = strlen(lpString);

	char* lpData = (char*)VirtualAlloc(NULL, 8+iSize, MEM_COMMIT, PAGE_READWRITE);	   //1080
    DWORD dwType = DD_TEXT;

	// copy data size to first 4 byte
	lpData[0] = (iSize & 255);
	lpData[1] = (iSize & 65535) >> 8;
	lpData[2] = iSize >> 16;
	lpData[3] = iSize >> 24;

	// copy data type  TODO
	lpData[4] = (dwType & 255);
	lpData[5] = (dwType & 65535) >> 8;
	lpData[6] = dwType >> 16;
	lpData[7] = dwType >> 24;

	for(int i=0;i<iSize;i++)
		lpData[8+i] = *(lpString+i);


	// send data	
	if(SOCKET_ERROR == send(sock, lpData, 8+iSize, 0))
	{
		DWORD dwErr = WSAGetLastError();
		TRACE1("\nFailed to send data error #%i", dwErr);
	}

}
