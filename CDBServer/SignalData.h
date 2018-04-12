#pragma once

#include <tbb\concurrent_hash_map.h>
using namespace tbb;

typedef void(*PTIoRequestSuccess)(DWORD dwTranstion, void* key, void* buf);
typedef void(*PTIoRequestFailed)(void* key, void* buf);

typedef bool(*PTAPIResponse)(void* bobj);

typedef struct _buffer_obj
{
public:
	WSAOVERLAPPED ol;
	PTIoRequestFailed pfnFailed;
	PTIoRequestSuccess pfnSuccess;
	PTAPIResponse pfndoApiResponse;
	struct _socket_obj* pRelatedSObj;
	_RecordsetPtr pRecorder;
	int nRecSetCount;
	WSABUF wsaBuf;
	DWORD dwRecvedCount;
	DWORD dwSendedCount;
	int nCmd;
	int nSubCmd;
	std::string strJrhm;// 使用jrhm时保存jrhm，使用iccid时保存iccid
	int datalen;
	TCHAR data[1];

public:
	void init(DWORD usefull_space)
	{
		memset(&ol, 0x00, sizeof(ol));
		pfnFailed = NULL;
		pfnSuccess = NULL;
		pfndoApiResponse = NULL;
		pRelatedSObj = NULL;
		ReleaseRecorder();
		nRecSetCount = 0;
		dwRecvedCount = 0;
		dwSendedCount = 0;
		nCmd = 0;
		nSubCmd = 0;
		strJrhm = "";
		datalen = usefull_space;
		memset(data, 0x00, usefull_space);
	}

	void SetIoRequestFunction(PTIoRequestFailed _pfnFailed, PTIoRequestSuccess _pfnSuccess)
	{
		pfnFailed = _pfnFailed;
		pfnSuccess = _pfnSuccess;
	}

	void ReleaseRecorder()
	{
		if (pRecorder)
		{
			if (pRecorder->State == adStateOpen)
				pRecorder->Close();
			pRecorder.Release();
			pRecorder = NULL;
		}
	}
}BUFFER_OBJ;
#define SIZE_OF_BUFFER_OBJ sizeof(BUFFER_OBJ)

typedef struct _buffer_obj_t
{
public:
	WSAOVERLAPPED ol;
	PTIoRequestFailed pfnFailed;
	PTIoRequestSuccess pfnSuccess;
	PTAPIResponse pfndoApiResponse;
	struct _socket_obj* pRelatedSObj;
	_RecordsetPtr pRecorder;
	int nRecSetCount;
	WSABUF wsaBuf;
	DWORD dwRecvedCount;
	DWORD dwSendedCount;
	int nCmd;
	int nSubCmd;
	std::string strJrhm;
	int datalen;
	//	TCHAR data[1];
}BUFFER_OBJ_T;
#define SIZE_OF_BUFFER_OBJ_T sizeof(BUFFER_OBJ_T)

typedef struct _socket_obj
{
public:
	SOCKET sock;
	struct _buffer_obj* pRelatedBObj;
	ADDRINFOT *sAddrInfo;
	int nKey;
	DWORD dwTick;
public:
	void init()
	{
		sock = INVALID_SOCKET;
		pRelatedBObj = NULL;
		sAddrInfo = NULL;
		nKey = 0;
		dwTick = 0;
	}
}SOCKET_OBJ;
#define SIZE_OF_SOCKET_OBJ sizeof(SOCKET_OBJ)

typedef struct _listen_obj
{
public:
	SOCKET sListenSock;
	HANDLE hPostAcceptExEvent;
	
	concurrent_hash_map<int, SOCKET_OBJ*> AcptMap;

	~_listen_obj()
	{
		if (INVALID_SOCKET != sListenSock)
		{
			closesocket(sListenSock);
			sListenSock = INVALID_SOCKET;
			if (NULL != hPostAcceptExEvent)
			{
				CloseHandle(hPostAcceptExEvent);
				hPostAcceptExEvent = NULL;
			}
		}
	}
public:
	void init()
	{
		sListenSock = INVALID_SOCKET;
		hPostAcceptExEvent = NULL;
		AcptMap.clear();
	}

	void InsertAcpt(SOCKET_OBJ* sobj)
	{
		concurrent_hash_map<int, SOCKET_OBJ*>::accessor a_ad;
		AcptMap.insert(a_ad, sobj->nKey);
		a_ad->second = sobj;
	}

	void RemoveAcpt(SOCKET_OBJ* sobj)
	{
		concurrent_hash_map<int, SOCKET_OBJ*>::accessor a_rm;
		if (AcptMap.find(a_rm, sobj->nKey))
			AcptMap.erase(a_rm);
	}
}LISTEN_OBJ;

bool UniqueInstance();

int GetRand();

extern LPFN_ACCEPTEX lpfnAccpetEx;
extern LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
extern LPFN_CONNECTEX lpfnConnectEx;

extern HANDLE hCompPort;
extern DWORD g_dwPageSize;