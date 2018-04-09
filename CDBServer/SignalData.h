#pragma once

typedef void(*PTIoRequestSuccess)(DWORD dwTranstion, void* key, void* buf);
typedef void(*PTIoRequestFailed)(void* key, void* buf);

typedef struct _buffer_obj
{
public:
	WSAOVERLAPPED ol;
	PTIoRequestFailed pfnFailed;
	PTIoRequestSuccess pfnSuccess;
	struct _socket_obj* pRelatedSObj;
	_RecordsetPtr pRecorder;
	WSABUF wsaBuf;
	DWORD dwRecvedCount;
	DWORD dwSendedCount;
	int datalen;
	TCHAR data[1];

public:
	void init(DWORD usefull_space)
	{
		memset(&ol, 0x00, sizeof(ol));
		pfnFailed = NULL;
		pfnSuccess = NULL;
		pRelatedSObj = NULL;
		ReleaseRecorder();
		dwRecvedCount = 0;
		dwSendedCount = 0;
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
	struct _socket_obj* pRelatedSObj;
	_RecordsetPtr pRecorder;
	WSABUF wsaBuf;
	DWORD dwRecvedCount;
	DWORD dwSendedCount;
	int datalen;
	//	TCHAR data[1];
}BUFFER_OBJ_T;
#define SIZE_OF_BUFFER_OBJ_T sizeof(BUFFER_OBJ_T)

typedef struct _socket_obj
{
public:
	SOCKET sock;
	struct _buffer_obj* pRelatedBObj;
public:
	void init()
	{
		sock = INVALID_SOCKET;
		pRelatedBObj = NULL;
	}
}SOCKET_OBJ;
#define SIZE_OF_SOCKET_OBJ sizeof(SOCKET_OBJ)

typedef struct _listen_obj
{
public:
	SOCKET sListenSock;
	DWORD dwAcceptExPendingCount;
	HANDLE hPostAcceptExEvent;

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
			dwAcceptExPendingCount = 0;
		}
	}
public:
	void init()
	{
		sListenSock = INVALID_SOCKET;
		dwAcceptExPendingCount = 0;
		hPostAcceptExEvent = NULL;
	}

	/*void AddBObj2AccpetExPendingList(struct _buffer_obj* obj)
	{
		InterlockedIncrement(&dwAcceptExPendingCount);
	}

	void DeleteBObjFromAccpetExPendingList(struct _buffer_obj* obj)
	{
		InterlockedDecrement(&dwAcceptExPendingCount);
	}*/
}LISTEN_OBJ;

bool UniqueInstance();

extern LPFN_ACCEPTEX lpfnAccpetEx;
extern LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;

extern HANDLE hCompPort;
extern DWORD g_dwPageSize;