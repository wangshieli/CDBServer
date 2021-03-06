#include "stdafx.h"
#include "SignalData.h"
#include "MemPool.h"
#include <vector>

using std::vector;

CRITICAL_SECTION csSObj;
CRITICAL_SECTION csBObj;

vector<SOCKET_OBJ*> vctSObj;
vector<BUFFER_OBJ*> vctBObj;

void InitMemPool()
{
	InitializeCriticalSection(&csSObj);
	InitializeCriticalSection(&csBObj);
	vctSObj.clear();
	vctBObj.clear();
}

SOCKET_OBJ* allocSObj()
{
	SOCKET_OBJ* obj = NULL;

	EnterCriticalSection(&csSObj);
	if (vctSObj.empty())
		obj = (SOCKET_OBJ*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SIZE_OF_SOCKET_OBJ);
	else
	{
		obj = vctSObj.back();
		vctSObj.pop_back();
	}
	LeaveCriticalSection(&csSObj);

	if (obj)
		obj->init();

	return obj;
}

void freeSObj(SOCKET_OBJ* obj)
{
	EnterCriticalSection(&csSObj);
	if (vctSObj.size() < 1000)
		vctSObj.push_back(obj);
	else
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, obj);
	LeaveCriticalSection(&csSObj);
}

BUFFER_OBJ* allocBObj(DWORD nSize)
{
	BUFFER_OBJ* obj = NULL;
	EnterCriticalSection(&csBObj);
	if (vctBObj.empty())
		obj = (BUFFER_OBJ*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nSize);
	else
	{
		obj = vctBObj.back();
		vctBObj.pop_back();
	}
	LeaveCriticalSection(&csBObj);

	if (obj)
		obj->init(nSize - SIZE_OF_BUFFER_OBJ_T);

	return obj;
}

void freeBObj(BUFFER_OBJ* obj)
{
	obj->ReleaseRecorder();
	obj->ReleaseRes();
	EnterCriticalSection(&csBObj);
	if (vctBObj.size() < 1000)
		vctBObj.push_back(obj);
	else
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, obj);
	LeaveCriticalSection(&csBObj);
}