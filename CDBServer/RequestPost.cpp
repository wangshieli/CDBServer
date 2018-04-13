#include "stdafx.h"
#include "SignalData.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "RequestComplete.h"
#include "NC_RequestComplete.h"

bool PostAcceptEx(LISTEN_OBJ* lobj, int nIndex)
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
	if (nIndex == 0)
		c_bobj->SetIoRequestFunction(AcceptCompFailed, AcceptCompSuccess);
	else 
		c_bobj->SetIoRequestFunction(NC_AcceptCompFailed, NC_AcceptCompSuccess);

	c_sobj->nKey = GetRand();
	lobj->InsertAcpt(c_sobj);

	bool brt = lpfnAccpetEx(lobj->sListenSock,
		c_sobj->sock, c_bobj->data,
		c_bobj->datalen - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, &c_bobj->ol);
	if (!brt)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			_tprintf(_T("acceptex Ê§°Ü\n"));
			lobj->RemoveAcpt(c_sobj);
			CSCloseSocket(c_sobj);
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

void CSCloseSocket(SOCKET_OBJ* _sobj)
{
	if (INVALID_SOCKET != _sobj->sock)
	{
		closesocket(_sobj->sock);
		_sobj->sock = INVALID_SOCKET;
	}
}