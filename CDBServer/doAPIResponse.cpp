#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doAPIResponse.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "API_RequestComplete.h"
#include "DealHeadTail.h"
#include "tinyxml2.h"

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
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(3);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(0);

	DealTail(sbuf, bobj);
}

TCHAR* Utf8ConvertAnsi(const TCHAR* strIn, int inLen)
{
	//int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strIn, -1, NULL, 0); // 如果字符串不是以空字符中止，设置为-1可能失败，可能成功
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strIn, inLen, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, strIn, inLen, (LPWSTR)pUnicode, unicodeLen);
	int targetLen = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pUnicode, -1, NULL, 0, NULL, NULL);
	BYTE* pTargetData = new BYTE[targetLen + 1];
	memset(pTargetData, 0, targetLen + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pUnicode, -1, (char*)pTargetData, targetLen, NULL, NULL);

	delete pUnicode;
	return (TCHAR*)pTargetData;
}

void DoReturnData(BUFFER_OBJ* bobj)
{
	if (!bobj->pfndoApiResponse(bobj))
	{
		API_Failed(bobj);
	}
}

bool doDisNumberResponse(void* _bobj)
{
	//static const char* test = "<?xml version = \"1.0\" encoding = \"utf-8\"?>"
	//	"<businessServiceResponse>" // 业务根节点
	//	"<RspType>10</RspType>"	// 请求状态 返回0，标识请求成功
	//	"<result>11</result>"	// 状态响应码 返回0，标识成功接收消息
	//	"<resultMsg>成功接收消息</resultMsg>"	// 返回消息信息
	//	"<GROUP_TRANSACTIONID>1000000252201606149170517340</GROUP_TRANSACTIONID>" // 流水号 请求流水
	//	"</businessServiceResponse>";

	BUFFER_OBJ* bobj = (BUFFER_OBJ*)_bobj;
	TCHAR* pResponData = Utf8ConvertAnsi(bobj->data, bobj->dwRecvedCount);
	tinyxml2::XMLDocument doc;
	if (tinyxml2::XML_SUCCESS != doc.Parse(pResponData))
	{
		// 不是XML格式的数据
		_tprintf(_T("%s\n"), pResponData);
		return false;
	}

	tinyxml2::XMLElement* RootElment = doc.FirstChildElement("businessServiceResponse");
	if (NULL == RootElment)
	{
		_tprintf(_T("接收到的xml根节点格式错误\n"));
		return false;
	}

	tinyxml2::XMLElement* pRspType = RootElment->FirstChildElement("RspType");
	if (NULL == pRspType)
	{
		_tprintf(_T("接收到的xml节点RspType格式错误\n"));
		return false;
	}
	const TCHAR* pcRspType = pRspType->GetText();
	
	tinyxml2::XMLElement* presult = pRspType->NextSiblingElement("result");
	if (NULL == presult)
	{
		_tprintf(_T("接收到的xml节点result格式错误\n"));
		return false;
	}
	const TCHAR* pcresult = presult->GetText();

	tinyxml2::XMLElement* presultMsg = presult->NextSiblingElement("resultMsg");
	if (NULL == presultMsg)
	{
		_tprintf(_T("接收到的xml节点resultMsg格式错误\n"));
		return false;
	}
	const TCHAR* pcresultMsg = presultMsg->GetText();

	tinyxml2::XMLElement* pGROUP_TRANSACTIONID = presultMsg->NextSiblingElement("GROUP_TRANSACTIONID");
	if (NULL == pGROUP_TRANSACTIONID)
	{
		_tprintf(_T("接收到的xml节点GROUP_TRANSACTIONID格式错误\n"));
		return false;
	}
	const TCHAR* pcGROUP_TRANSACTIONID = pGROUP_TRANSACTIONID->GetText();

	return true;
}