#include "stdafx.h"
#include "IoWorkerThread.h"
#include "SignalData.h"

unsigned int _stdcall completionRoution(LPVOID)
{
	ULONG_PTR key;
	BUFFER_OBJ* bobj;
	LPOVERLAPPED lpol;
	DWORD dwTranstion;
	BOOL bSuccess;

	while (true)
	{
		bSuccess = GetQueuedCompletionStatus(hCompPort, &dwTranstion, &key, &lpol, INFINITE);
		if (NULL == lpol)
		{
			printf("如果不是认为设定NULL为退出信号，那么就是发生重大错误，直接退出\n");
			return 0;
		}

		bobj = CONTAINING_RECORD(lpol, BUFFER_OBJ, ol);

		if (!bSuccess)
			bobj->pfnFailed((void*)key, bobj);
		else
			bobj->pfnSuccess(dwTranstion, (void*)key, bobj);
	}

	return 0;
}