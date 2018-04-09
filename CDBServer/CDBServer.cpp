// CDBServer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SignalData.h"
#include "InitWinsock.h"
#include "MemPool.h"
#include "DBPool.h"
#include "Database_map.h"
#include "IoWorkerThread.h"
#include "RequestPost.h"

#ifdef _DEBUG
#define SERVER_PORT	6088
#else
#define SERVER_PORT 6086
#endif // _DEBUG


int main()
{
	if (!UniqueInstance())
	{
		_tprintf(_T("已经有服务器处于工作状态，不要重复启动\n"));
		Sleep(1000 * 20);
		return -1;
	}

	if (!InitWinsock2() || !GetExtensionFunctionPointer() || !InitAdoMysql())
		return false;

	InitDatabase_map();
	InitMemPool();

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwNumberOfCPU = sys.dwNumberOfProcessors;
	g_dwPageSize = sys.dwPageSize * 4;

	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
	if (NULL == hCompPort)
	{
		_tprintf(_T("创建完成端口失败, errCode = %d\n"), WSAGetLastError());
		WSACleanup();
		return -1;
	}

	int nThreadCount = 0;
	for (DWORD i = 0; i < dwNumberOfCPU; i++)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, completionRoution, NULL, 0, NULL);
		if (NULL != hCompPort)
		{
			CloseHandle(hThread);
			nThreadCount++;
		}
	}
	if (nThreadCount <= 0)
	{
		_tprintf(_T("没有创建有效的工作线程\n"));
		CloseHandle(hCompPort);
		WSACleanup();
		return -1;
	}

	LISTEN_OBJ *lobj = new LISTEN_OBJ();
	if (NULL == lobj)
	{
		_tprintf(_T("分配监听socket对象失败, errCode = %d\n"), WSAGetLastError());
		CloseHandle(hCompPort);
		WSACleanup();
		return -1;
	}
	lobj->init();

	lobj->sListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == lobj->sListenSock)
	{
		_tprintf(_T("创建监听socket失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	struct sockaddr_in laddr;
	memset(&laddr, 0x00, sizeof(laddr));
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(SERVER_PORT);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(lobj->sListenSock, (SOCKADDR*)&laddr, sizeof(laddr)))
	{
		_tprintf(_T("监听socket绑定失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	// 创建事件，完成自动处理资源分配
	lobj->hPostAcceptExEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == lobj->hPostAcceptExEvent)
	{
		_tprintf(_T("监听事件创建失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (SOCKET_ERROR == WSAEventSelect(lobj->sListenSock, lobj->hPostAcceptExEvent, FD_ACCEPT))
	{
		_tprintf(_T("监听事件关联失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (NULL == CreateIoCompletionPort((HANDLE)lobj->sListenSock, hCompPort, (ULONG_PTR)lobj, 0))
	{
		_tprintf(_T("监听socket关联完成端口失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (SOCKET_ERROR == listen(lobj->sListenSock, SOMAXCONN))
	{
		_tprintf(_T("socket监听失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	for (size_t i = 0; i < 2; i++)
	{
		PostAcceptEx(lobj);
	}

	while (true)
	{
		int rt = WaitForSingleObject(lobj->hPostAcceptExEvent, INFINITE);
		if (WSA_WAIT_FAILED == rt)
		{
			_tprintf(_T("带来异常，推出\n"));
			return -1;
		}

		for (size_t i = 0; i < 2; i++)
		{
			PostAcceptEx(lobj);
		}
	}

    return 0;

error:
	delete lobj;
	CloseHandle(hCompPort);
	WSACleanup();
	system("pause");
	return -1;
}

