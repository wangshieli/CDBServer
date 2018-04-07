#include "stdafx.h"
#include "SignalData.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "RequestComplete.h"

bool PostAcceptEx(LISTEN_OBJ* lobj)
{
	DWORD dwBytes = 0;
	SOCKET_OBJ* c_sobj = NULL;
	BUFFER_OBJ* c_bobj = NULL;

	c_bobj = allocBObj(g_dwPageSize);
	if (NULL == c_bobj)
		return false;

	c_sobj = allocSObj();
	if (NULL == c_sobj)
	{
		freeBObj(c_bobj);
		return false;
	}

	c_sobj->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == c_sobj->sock)
	{
		freeBObj(c_bobj);
		freeSObj(c_sobj);
		return false;
	}

	c_sobj->pRelatedBObj = c_bobj;
	c_bobj->pRelatedSObj = c_sobj;
	c_bobj->SetIoRequestFunction(AcceptCompFailed, AcceptCompSuccess);

	bool brt = lpfnAccpetEx(lobj->sListenSock,
		c_sobj->sock, c_bobj->data,
		c_bobj->datalen - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, &c_bobj->ol);
	if (!brt)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			_tprintf(_T("acceptex Ê§°Ü\n"));
			closesocket(c_sobj->sock);
			freeBObj(c_bobj);
			freeSObj(c_sobj);
			return false;
		}
	}

	return true;
}

BOOL PostZeroRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj)
{
	DWORD dwBytes = 0,
		dwFlags = 0;

	int err = 0;

	_bobj->wsaBuf.buf = NULL;
	_bobj->wsaBuf.len = 0;

	err = WSARecv(_sobj->sock, &_bobj->wsaBuf, 1, &dwBytes, &dwFlags, &_bobj->ol, NULL);
	if (SOCKET_ERROR == err && WSA_IO_PENDING != WSAGetLastError())
		return FALSE;

	return TRUE;
}

BOOL PostRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj)
{
	DWORD dwBytes = 0,
		dwFlags = 0;

	int err = 0;

	_bobj->wsaBuf.buf = _bobj->data + _bobj->dwRecvedCount;
	_bobj->wsaBuf.len = _bobj->datalen - _bobj->dwRecvedCount;

	err = WSARecv(_sobj->sock, &_bobj->wsaBuf, 1, &dwBytes, &dwFlags, &_bobj->ol, NULL);
	if (SOCKET_ERROR == err && WSA_IO_PENDING != WSAGetLastError())
		return FALSE;

	return TRUE;
}

BOOL PostSend(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj)
{
	DWORD dwBytes = 0;
	int err = 0;

	_bobj->wsaBuf.buf = _bobj->data + _bobj->dwSendedCount;
	_bobj->wsaBuf.len = _bobj->dwRecvedCount - _bobj->dwSendedCount;

	err = WSASend(_sobj->sock, &_bobj->wsaBuf, 1, &dwBytes, 0, &_bobj->ol, NULL);
	if (SOCKET_ERROR == err && WSA_IO_PENDING != WSAGetLastError())
		return FALSE;

	return TRUE;
}