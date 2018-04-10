#include "stdafx.h"
#include "InitWinsock.h"

GUID GuidAcceptEx = WSAID_ACCEPTEX,
GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS,
GuidConnectEx = WSAID_CONNECTEX;

LPFN_ACCEPTEX lpfnAccpetEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
LPFN_CONNECTEX lpfnConnectEx = NULL;

bool InitWinsock2()
{
	int rt = 0;
	WSADATA wsadata;

	rt = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (0 != rt)
		return false;

	return true;
}

bool GetExtensionFunctionPointer()
{
	SOCKET sTemp = INVALID_SOCKET;
	sTemp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sTemp)
	{
		WSACleanup();
		return false;
	}

	DWORD dwBytes = 0;
	int rtErr = 0;

	rtErr = WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs),
		&lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs),
		&dwBytes, NULL, NULL);
	if (SOCKET_ERROR == rtErr)
		goto error;

	rtErr = WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAccpetEx, sizeof(lpfnAccpetEx),
		&dwBytes, NULL, NULL);
	if (SOCKET_ERROR == rtErr)
		goto error;

	rtErr = WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx, sizeof(GuidConnectEx),
		&lpfnConnectEx, sizeof(lpfnConnectEx),
		&dwBytes, NULL, NULL);
	if (SOCKET_ERROR == rtErr)
		goto error;

	closesocket(sTemp);
	sTemp = INVALID_SOCKET;

	return true;

error:
	closesocket(sTemp);
	WSACleanup();
	return false;
}