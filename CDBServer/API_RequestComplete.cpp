#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "API_RequestComplete.h"
#include "MemPool.h"
#include "RequestPost.h"
#include "ProcessCommand.h"
#include "DealHeadTail.h"

concurrent_hash_map<int, SOCKET_OBJ*> ConnMap;

void InsertConn(SOCKET_OBJ* sobj)
{
	concurrent_hash_map<int, SOCKET_OBJ*>::accessor a_ad;
	ConnMap.insert(a_ad, sobj->nKey);
	a_ad->second = sobj;
}

void RemoveConn(SOCKET_OBJ* sobj)
{
	concurrent_hash_map<int, SOCKET_OBJ*>::accessor a_rm;
	if (ConnMap.find(a_rm, sobj->nKey))
		ConnMap.erase(a_rm);
}

unsigned int _stdcall checkconnectex(LPVOID pVoid)
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	while (WaitForSingleObject(hEvent, 60 * 1000) == WAIT_TIMEOUT)
	{
		DWORD dwCurTick = GetTickCount();
		for (concurrent_hash_map<int, SOCKET_OBJ*>::iterator i = ConnMap.begin(); i != ConnMap.end(); ++i)
		{
			if (dwCurTick - i->second->dwTick > 60 * 1000)
				CSCloseSocket(i->second);
		}
	}

	return 0;
}

bool doApi(BUFFER_OBJ* bobj)
{
	int nError = 0;
	DWORD dwBytes = 0;

	ADDRINFOT hints, *sAddrInfo = NULL;
	SOCKET_OBJ* a_sobj = NULL;

	a_sobj = allocSObj();
	if (NULL == a_sobj)
		goto error;

	memset(&hints, 0x00, sizeof(hints));
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	nError = GetAddrInfo(_T("api.ct10649.com"), _T("9001"), &hints, &sAddrInfo);
	if (0 != nError)
		goto error;

	//char ipbuf[16];
	//struct addrinfo *res, *cur;
	//for (cur = sAddrInfo; cur != NULL; cur = cur->ai_next) {
	//	struct sockaddr_in* addr = (struct sockaddr_in *)cur->ai_addr;
	//	printf("%s\n", inet_ntop(AF_INET,&addr->sin_addr, ipbuf, 16));
	//}

	a_sobj->sAddrInfo = sAddrInfo;
	a_sobj->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == a_sobj->sock)
		goto error;

	struct sockaddr_in taddr;
	taddr.sin_family = AF_INET;
	taddr.sin_port = htons(0);
	taddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (SOCKET_ERROR == bind(a_sobj->sock, (sockaddr*)&taddr, sizeof(taddr)))
		goto error;

	if (NULL == CreateIoCompletionPort((HANDLE)a_sobj->sock, hCompPort, (ULONG_PTR)a_sobj, 0))
		goto error;

	bobj->SetIoRequestFunction(API_ConnectCompFailed, API_ConnectCompSuccess);
	a_sobj->pRelatedBObj = bobj;

	a_sobj->nKey = GetRand();
	a_sobj->dwTick = GetTickCount();
	InsertConn(a_sobj);

	if (!lpfnConnectEx(a_sobj->sock, (sockaddr*)sAddrInfo->ai_addr, sAddrInfo->ai_addrlen, bobj->data, bobj->dwRecvedCount, &dwBytes, &bobj->ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			RemoveConn(a_sobj);
			goto error;
		}
	}

	return true;

error:
	if (NULL != a_sobj)
	{
		CSCloseSocket(a_sobj);
		if (NULL != a_sobj->sAddrInfo)
			FreeAddrInfo(a_sobj->sAddrInfo);
		freeSObj(a_sobj);
	}

	API_Failed(bobj);

	return false;
}

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

void UTF8ToGBK(TCHAR* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	unsigned short * wszGBK = new unsigned short[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)strUTF8, -1, (LPWSTR)wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
	
	std::string strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	_tprintf(_T("%s\n"), strTemp.c_str());
}

void API_RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return API_RecvCompFailed(_sobj, _bobj);

	SOCKET_OBJ* a_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwRecvedCount += dwTransion;
	c_bobj->SetIoRequestFunction(API_CheckRecvFailed, API_CheckRecvSuccess);

	if (PostRecv(a_sobj, c_bobj)) // 接收判断
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

	if (dwTransion == 0) // 对方关闭连接
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

void API_Failed(BUFFER_OBJ* bobj)
{
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(3);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(1);

	DealTail(sbuf, bobj);
}

void API_Successed(BUFFER_OBJ* bobj)
{
	
}

void DoReturnData(BUFFER_OBJ* bobj)
{
	if (1)
	{
		API_Successed(bobj);
	}
	else
	{
		API_Failed(bobj);
	}
}