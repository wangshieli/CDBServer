#include "stdafx.h"
#include "SignalData.h"
#include "NC_RequestComplete.h"
#include "MemPool.h"
#include "RequestPost.h"

extern struct tcp_keepalive alive_in;
extern struct tcp_keepalive alive_out;
extern unsigned long ulBytesReturn;

void NC_AcceptCompFailed(void* _lobj, void* _c_obj)
{
	LISTEN_OBJ* lobj = (LISTEN_OBJ*)_lobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_c_obj;
	SOCKET_OBJ* c_sobj = c_bobj->pRelatedSObj;

	lobj->RemoveAcpt(c_sobj);

	CSCloseSocket(c_sobj);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void NC_AcceptCompSuccess(DWORD dwTranstion, void* _lobj, void* _c_bobj)
{
	if (dwTranstion <= 0)
		return NC_AcceptCompFailed(_lobj, _c_bobj);

	LISTEN_OBJ* lobj = (LISTEN_OBJ*)_lobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_c_bobj;
	SOCKET_OBJ* c_sobj = c_bobj->pRelatedSObj;

	lobj->RemoveAcpt(c_sobj);

	if (SOCKET_ERROR == WSAIoctl(c_sobj->sock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
		&alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL))
		_tprintf(_T("设置客户端连接心跳检测失败, errCode = %d\n"), WSAGetLastError());

	if (NULL == CreateIoCompletionPort((HANDLE)c_sobj->sock, hCompPort, (ULONG_PTR)c_sobj, 0))
	{
		_tprintf(_T("客户端socket绑定完成端口失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	c_bobj->dwRecvedCount += dwTranstion;

	SOCKADDR* localAddr,
		*remoteAddr;
	localAddr = NULL;
	remoteAddr = NULL;
	int localAddrlen,
		remoteAddrlen;

	lpfnGetAcceptExSockaddrs(c_bobj->data, c_bobj->datalen - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&localAddr, &localAddrlen,
		&remoteAddr, &remoteAddrlen);

	if (NULL == memchr(c_bobj->data + c_bobj->dwRecvedCount - 1, 0x0d, 1))
	{
		c_bobj->SetIoRequestFunction(NC_RecvZeroCompFailed, NC_RecvZeroCompSuccess);
		if (!PostZeroRecv(c_sobj, c_bobj))
		{
			_tprintf(_T("客户端信息接收失败, errCode = %d\n"), WSAGetLastError());
			goto error;
		}
	}
	else
	{
		//ProcessCommand(c_bobj);
	}

	return;

error:
	CSCloseSocket(c_sobj);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
	return;
}

void NC_RecvZeroCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(c_sobj);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void NC_RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->SetIoRequestFunction(NC_RecvCompFailed, NC_RecvCompSuccess);
	if (!PostRecv(c_sobj, c_bobj))
	{
		CSCloseSocket(c_sobj);
		freeSObj(c_sobj);
		freeBObj(c_bobj);
		return;
	}
}

void NC_RecvCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(c_sobj);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void NC_RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return NC_RecvCompFailed(_sobj, _bobj);

	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwRecvedCount += dwTransion;
	if (NULL == memchr(c_bobj->data + c_bobj->dwRecvedCount - 1, 0x0d, 1))
	{
		c_bobj->SetIoRequestFunction(NC_RecvZeroCompFailed, NC_RecvZeroCompSuccess);
		if (!PostZeroRecv(c_sobj, c_bobj))
		{
			CSCloseSocket(c_sobj);
			freeSObj(c_sobj);
			freeBObj(c_bobj);
			return;
		}
	}
	else
	{
		//ProcessCommand(c_bobj);
	}
}

void NC_SendCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(c_sobj);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void NC_SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return NC_SendCompFailed(_sobj, _bobj);

	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwSendedCount += dwTransion;
	if (c_bobj->dwSendedCount < c_bobj->dwRecvedCount)
	{
		if (!PostSend(c_sobj, c_bobj))
		{
			CSCloseSocket(c_sobj);
			freeSObj(c_sobj);
			freeBObj(c_bobj);
			return;
		}
		return;
	}

	c_bobj->dwRecvedCount = 0;
	c_bobj->dwSendedCount = 0;
	c_bobj->SetIoRequestFunction(NC_RecvZeroCompFailed, NC_RecvZeroCompSuccess);
	if (!PostZeroRecv(c_sobj, c_bobj))
	{
		CSCloseSocket(c_sobj);
		freeSObj(c_sobj);
		freeBObj(c_bobj);
		return;
	}
}