#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "API_RequestComplete.h"
#include "MemPool.h"
#include "RequestPost.h"
#include "doAPIResponse.h"

void API_ConnectCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	RemoveConn(a_sobj);
	FreeAddrInfo(a_sobj->sAddrInfo);

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);

	API_Failed(c_bobj);
}

void API_ConnectCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return API_ConnectCompFailed(_sobj, _bobj);

	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	RemoveConn(a_sobj);
	FreeAddrInfo(a_sobj->sAddrInfo);

	c_bobj->dwSendedCount += dwTransion;
	if (c_bobj->dwSendedCount < c_bobj->dwRecvedCount)
	{
		c_bobj->SetIoRequestFunction(API_SendCompFailed, API_SendCompSuccess);
		if (!PostSend(a_sobj, c_bobj))
		{
			CSCloseSocket(a_sobj);
			freeSObj(a_sobj);
			goto error; 
		}
		return;
	}

	c_bobj->dwRecvedCount = 0;
	c_bobj->dwSendedCount = 0;
	c_bobj->SetIoRequestFunction(API_RecvZeroCompFailed, API_RecvZeroCompSuccess);
	if (!PostZeroRecv(a_sobj, c_bobj))
	{
		CSCloseSocket(a_sobj);
		freeSObj(a_sobj);
		goto error;;
	}

	return;
error:
	API_Failed(c_bobj);
	return;
}

void API_SendCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);

	API_Failed(c_bobj);
}

void API_SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return API_SendCompFailed(_sobj, _bobj);

	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwSendedCount += dwTransion;
	if (c_bobj->dwSendedCount < c_bobj->dwRecvedCount)
	{
		if (!PostSend(a_sobj, c_bobj))
		{
			CSCloseSocket(a_sobj);
			freeSObj(a_sobj);
			goto error;
		}
		return;
	}

	c_bobj->dwRecvedCount = 0;
	c_bobj->dwSendedCount = 0;
	c_bobj->SetIoRequestFunction(API_RecvZeroCompFailed, API_RecvZeroCompSuccess);
	if (!PostZeroRecv(a_sobj, c_bobj))
	{
		CSCloseSocket(a_sobj);
		freeSObj(a_sobj);
		goto error;;
	}
	return;

error:
	API_Failed(c_bobj);
	return;
}

void API_RecvZeroCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);

	API_Failed(c_bobj);
}

void API_RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->SetIoRequestFunction(API_RecvCompFailed, API_RecvCompSuccess);
	if (!PostRecv(c_sobj, c_bobj))
	{
		CSCloseSocket(c_sobj);
		freeSObj(c_sobj);
		goto error;
	}
	return;

error:
	API_Failed(c_bobj);
	return;
}

void API_RecvCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);

	API_Failed(c_bobj);
}

void API_RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return API_RecvCompFailed(_sobj, _bobj);

	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwRecvedCount += dwTransion;
	c_bobj->SetIoRequestFunction(API_CheckRecvFailed, API_CheckRecvSuccess);

	if (PostRecv(a_sobj, c_bobj)) // �����ж�
		return;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);
	DoReturnData(c_bobj);
}

void API_CheckRecvFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);
	DoReturnData(c_bobj);
}

void API_CheckRecvSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	if (dwTransion == 0) // �Է��ر�����
	{
		CSCloseSocket(a_sobj);
		freeSObj(a_sobj);
		DoReturnData(c_bobj);
		return;
	}

	c_bobj->dwRecvedCount += dwTransion;
	if (PostRecv(a_sobj, c_bobj))
		return;

	CSCloseSocket(a_sobj);
	freeSObj(a_sobj);
	DoReturnData(c_bobj);
}