#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doAPIResponse.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "API_RequestComplete.h"
#include "DealHeadTail.h"
#include "theHelpThread.h"
#include "tinyxml2.h"
#include "cJSON.h"

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

	_msgpack.pack_array(4);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(1);
	_msgpack.pack(bobj->strJrhm);

	DealTail(sbuf, bobj);
}

void API_Successed(BUFFER_OBJ* bobj)
{
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(4);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(0);
	_msgpack.pack(bobj->strJrhm);

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
		return API_Failed(bobj);
	}
}


//	static const char* test = "<?xml version = \"1.0\" encoding = \"utf-8\"?>"
//	"<businessServiceResponse>" // 业务根节点
//		"<RspType>0</RspType>"	// 请求状态 返回0，标识请求成功
//		"<result>0</result>"	// 状态响应码 返回0，标识成功接收消息
//		"<resultMsg>成功接收消息</resultMsg>"	// 返回消息信息
//		"<GROUP_TRANSACTIONID>1000000252201606149170517340</GROUP_TRANSACTIONID>" // 流水号 请求流水
//	"</businessServiceResponse>";
bool doDisNumberResponse(void* _bobj)
{
	BUFFER_OBJ* bobj = (BUFFER_OBJ*)_bobj;
	TCHAR* pResponData = Utf8ConvertAnsi(bobj->data, bobj->dwRecvedCount);
	tinyxml2::XMLDocument doc;
	if (tinyxml2::XML_SUCCESS != doc.Parse(pResponData))
	{
		delete pResponData;
		return false;
	}
	delete pResponData;

	DIS_NUMBER* pds = new DIS_NUMBER;
	pds->strJrhm = bobj->strJrhm;
	tinyxml2::XMLElement* root = doc.RootElement();
	tinyxml2::XMLElement* RspType = root->FirstChildElement(); // <RspType>0</RspType>
	pds->strRsptype = RspType->GetText();

	tinyxml2::XMLElement* result = RspType->NextSiblingElement(); // <result>0</result>
	pds->strResult = result->GetText();

	tinyxml2::XMLElement* resultMsg = result->NextSiblingElement(); // <resultMsg>成功接收消息</resultMsg>
	pds->strResultMsg = resultMsg->GetText();

	tinyxml2::XMLElement* GROUP_TRANSACTIONID = resultMsg->NextSiblingElement(); // <GROUP_TRANSACTIONID>1000000252201606149170517340</GROUP_TRANSACTIONID>
	pds->strGROUP_TRANSACTIONID = GROUP_TRANSACTIONID->GetText();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(5);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(0);
	_msgpack.pack(bobj->strJrhm);
	_msgpack.pack_array(1);
	_msgpack.pack_array(4);
	_msgpack.pack(pds->strRsptype);
	_msgpack.pack(pds->strResult);
	_msgpack.pack(pds->strResultMsg);
	_msgpack.pack(pds->strGROUP_TRANSACTIONID);

	DealTail(sbuf, bobj);

	PostThreadMessage(g_HelpThreadID, MSG_DIS_NUMBER, (WPARAM)pds, 0);// 进行数据库操作

	return true;
}


//<?xml version = "1.0" encoding = "utf-8"?>
//<root>
//	<Query_response>
//		<BasicInfo>
//			<result>0</result>
//			<resultMsg>处理成功!</resultMsg>
//		</BasicInfo>
//		<prodRecords>
//			<prodRecord>
//<				productInfo>
//					<productStatusCd>1</productStatusCd>
//					<productStatusName>在用</productStatusName>
//					<servCreateDate>20160531</servCreateDate>
//				</productInfo>
//			</prodRecord>
//		</prodRecords>
//		<number>14910000000</number>
//		<GROUP_TRANSACTIONID>1000000252201609302104391983</GROUP_TRANSACTIONID>
//	</Query_response>
//</root>
bool doCardStatusResponse(void* _bobj)
{
	BUFFER_OBJ* bobj = (BUFFER_OBJ*)_bobj;
	TCHAR* pResponData = Utf8ConvertAnsi(bobj->data, bobj->dwRecvedCount);
	tinyxml2::XMLDocument doc;
	if (tinyxml2::XML_SUCCESS != doc.Parse(pResponData))
	{
		delete pResponData;
		return false;
	}
	delete pResponData;

	CARD_STATUS* pcs = new CARD_STATUS; // 需要释放
	tinyxml2::XMLElement* root = doc.RootElement();
	tinyxml2::XMLElement* BasicInfo = root->FirstChildElement()->FirstChildElement();
	tinyxml2::XMLElement* result = BasicInfo->FirstChildElement(); // <result>0</result>
	pcs->strResult = result->GetText();

	tinyxml2::XMLElement* resultMsg = result->NextSiblingElement(); // <resultMsg>处理成功!</resultMsg>
	pcs->strResultMsg = resultMsg->GetText();

	tinyxml2::XMLElement* prodRecords = BasicInfo->NextSiblingElement();
	tinyxml2::XMLElement* productInfo = prodRecords->FirstChildElement()->FirstChildElement();
	tinyxml2::XMLElement* productStatusCd = productInfo->FirstChildElement(); // <productStatusCd>1</productStatusCd>
	pcs->strProductStatusCd = productStatusCd->GetText();

	tinyxml2::XMLElement* productStatusName = productStatusCd->NextSiblingElement(); // <productStatusName>在用</productStatusName>
	pcs->strProductStatusName = productStatusName->GetText();

	tinyxml2::XMLElement* servCreateDate = productStatusName->NextSiblingElement(); // <servCreateDate>20160531</servCreateDate>
	pcs->strPservCreateDate = servCreateDate->GetText();

	tinyxml2::XMLElement* number = prodRecords->NextSiblingElement(); // <number>14910000000</number>
	pcs->strNumber = number->GetText();

	tinyxml2::XMLElement* GROUP_TRANSACTIONID = number->NextSiblingElement(); // <GROUP_TRANSACTIONID>1000000252201609302104391983</GROUP_TRANSACTIONID>
	pcs->strGROUP_TRANSACTIONID = GROUP_TRANSACTIONID->GetText();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(5);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(0);
	_msgpack.pack(bobj->strJrhm);
	_msgpack.pack_array(1);
	_msgpack.pack_array(6);
	_msgpack.pack(pcs->strResult);
	_msgpack.pack(pcs->strResultMsg);
	_msgpack.pack(pcs->strProductStatusCd);
	_msgpack.pack(pcs->strProductStatusName);
	_msgpack.pack(pcs->strPservCreateDate);
	//_msgpack.pack(pcs->strNumber);
	_msgpack.pack(pcs->strGROUP_TRANSACTIONID);

	DealTail(sbuf, bobj);

	PostThreadMessage(g_HelpThreadID, MSG_CARD_STATUS, (WPARAM)pcs, 0);// 进行数据库操作

	return true;
}

//{
//	"RESULT":   "0",
//	"SMSG" : "成功",
//	"GROUP_TRANSACTIONID" : "1000000252201702161611585211"
//}
bool doServActiveResponse(void* _bobj)
{
	BUFFER_OBJ* bobj = (BUFFER_OBJ*)_bobj;
	TCHAR* pResponData = Utf8ConvertAnsi(bobj->data, bobj->dwRecvedCount);
	cJSON* root = NULL;
	root = cJSON_Parse(pResponData);
	if (NULL == root)
	{
		delete pResponData;
		return false;
	}
	delete pResponData;

	SERV_ACTIVE* psa = new SERV_ACTIVE;
	psa->strJrhm = bobj->strJrhm;
	cJSON* RESULT = cJSON_GetObjectItem(root, "RESULT");
	psa->strResult = RESULT->valuestring;
	cJSON* SMSG = cJSON_GetObjectItem(root, "SMSG");
	psa->strSmsg = SMSG->valuestring;
	cJSON* GROUP_TRANSACTIONID = cJSON_GetObjectItem(root, "GROUP_TRANSACTIONID");
	psa->strGROUP_TRANSACTIONID = GROUP_TRANSACTIONID->valuestring;

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_msgpack.pack_array(5);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(0);
	_msgpack.pack(bobj->strJrhm);
	_msgpack.pack_array(1);
	_msgpack.pack_array(3);
	_msgpack.pack(psa->strResult);
	_msgpack.pack(psa->strSmsg);
	_msgpack.pack(psa->strGROUP_TRANSACTIONID);

	DealTail(sbuf, bobj);

	PostThreadMessage(g_HelpThreadID, MSG_SERV_ACTIVE, (WPARAM)psa, 0);

	cJSON_Delete(root);
	return true;
}

//{ 
//	"status":1,
//	"message" : "ok",
//	"url" : http ://202.102.116.186:7001/NetPay/ePayAction.do?RequestValue=CC000E94924805D69D86DC1F990B5480906DD522E95FB446ACC93992A8E1BA1C328680881E11662DFD315608AF3A2E90374FA1E836B0E6BCDDCB941EF0000008BC8A626518D9B5F319D7AAB4649580534E788E7A5BA3975BF06F447ED6C0CE372F7F2B0A1AFB304E87C6D0426A2EBEE5F32649D9AD1F091A092F18F14F5EE12D6FAE31B687DF0AA4882319F2D02E2BEF05878F8C5C7F150EC11A69CB17C8787ED6922F5D1362452C9DAE9829CE9AE4BA942A1053FA3F27CBA650435822CF50374BDD69A19BDEAD63B52B1608215BA2F7526E6059C7EEF424E023AFE94A2E0000F0EB6C3810B163726A33752183F57BF7577A57645734B02CBF86D99D071B7AAC0D2CFE1B090E1830BB7650000F492E54DD24E418AE617BDCB9FD1692A7AA2E5298F7191F7831098195248888D1D601847B65E8C428560536B9BFB78056ED40A2A73F50F72AA6C97C0D63AD3805DE49
//}
bool doPay2Response(void* bobj)
{
	return true;
}

//	<?xml version = "1.0" encoding = "utf-8"?>
//	<poolList>
//		<pool>
//			<create_date>2016-02-19</create_date>
//			<pool_infoUnit>GB</pool_infoUnit>
//			<state>在用</state>
//			<pool_info>10</pool_info>
//			<exp_date>3000-01-01</exp_date>
//			<acc_nbr>50LLC04244</acc_nbr>
//			<eff_date>2016-02-19</eff_date>
//		</pool>
//	</poolList>
bool doPoolListResponse(void* bobj)
{
	return true;
}