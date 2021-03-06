#include "stdafx.h"
#include "SignalData.h"
#include "doNcData.h"
#include "NC_RequestComplete.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "tinyxml2.h"
#include "theHelpThread.h"

extern TCHAR* Utf8ConvertAnsi(const TCHAR* strIn, int inLen);

bool DoNotifyContractRoot(tinyxml2::XMLElement* RootElment);
bool DoWanrningContractRoot(tinyxml2::XMLElement* RootElment);
void doNcResponse(BUFFER_OBJ* bobj);

bool doNcData(BUFFER_OBJ* bobj)
{
	TCHAR* pResponData = Utf8ConvertAnsi(bobj->data, bobj->dwRecvedCount);
	doNcResponse(bobj); // 返回推送反馈报文
	tinyxml2::XMLDocument doc;
	if (tinyxml2::XML_SUCCESS != doc.Parse(pResponData))
	{
		delete pResponData;
		return false;
	}
	delete pResponData;
	tinyxml2::XMLElement* RootElment = doc.RootElement();
	const TCHAR* rootName = RootElment->GetText();
	if (strcmp(rootName, "NotifyContractRoot") == 0)
	{
		return DoNotifyContractRoot(RootElment);
	}
	else if (strcmp(rootName, "NotifyContractRoot") == 0)
	{
		return DoWanrningContractRoot(RootElment);
	}

	return true;
}

//<?xml version = "1.0" encoding = "utf-8"?>
//<NotifyContractRoot>
//<!--报文类型 1：报竣 2：告警-->
//<TYPE>1< / TYPE>
//<!- 流水号->
//	<GROUP_TRANSACTIONID>1000000252201605131983898593< / GROUP_TRANSACTIONID>
//	<!- 报竣信息，比如：竣工，前端错误等，判断本次操作是否竣工也用这个节点->
//	<STATUSINFO>竣工< / STATUSINFO>
//	<!- 号码->
//	<ACCNBR>1064915901733< / ACCNBR>
//	<!- 请求发送时间->
//	<SENDDT>20160513153805< / SENDDT>
//	<!--操作类型，具体描述见本节第6节-->
//	<ACCEPTTYPE>8< / ACCEPTTYPE>
//	<!- 操作详情->
//	<ACCEPTMSG>号码:1064915901733，办理停机保号< / ACCEPTMSG>
//	<!- 响应时间->
//	<STATUSDT>20160513153905< / STATUSDT>
//	<!- 响应消息，如有错误也在这里记入, 本节点不作为判断竣工的依据->
//	<RESULTMSG>成功< / RESULTMSG>
//	< / NotifyContractRoot>

bool DoNotifyContractRoot(tinyxml2::XMLElement* root)
{
	NOTIFY_CONTRACT_ROOT* pNcr = new NOTIFY_CONTRACT_ROOT; // 需要释放
	tinyxml2::XMLElement* TYPE = root->FirstChildElement(); // <TYPE>1</TYPE> 
	pNcr->strType = TYPE->GetText();

	tinyxml2::XMLElement* GROUP_TRANSACTIONID = TYPE->NextSiblingElement(); // <GROUP_TRANSACTIONID>1000000252201605131983898593</GROUP_TRANSACTIONID>
	pNcr->strGROUP_TRANSACTIONID = GROUP_TRANSACTIONID->GetText();

	tinyxml2::XMLElement* STATUSINFO = GROUP_TRANSACTIONID->NextSiblingElement(); // <STATUSINFO>竣工</STATUSINFO>
	pNcr->strStatusinfo = STATUSINFO->GetText();

	tinyxml2::XMLElement* ACCNBR = STATUSINFO->NextSiblingElement(); // <ACCNBR>1064915901733</ACCNBR>
	pNcr->strAccnbr = ACCNBR->GetText();

	tinyxml2::XMLElement* SENDDT = ACCNBR->NextSiblingElement(); // <SENDDT>20160513153805</SENDDT>
	pNcr->strSenddt = SENDDT->GetText();

	tinyxml2::XMLElement* ACCEPTTYPE = SENDDT->NextSiblingElement(); // <ACCEPTTYPE>8</ACCEPTTYPE>
	pNcr->strAccepttype = ACCEPTTYPE->GetText();

	tinyxml2::XMLElement* ACCEPTMSG = ACCEPTTYPE->NextSiblingElement(); // <ACCEPTMSG>号码:1064915901733，办理停机保号</ACCEPTMSG>
	pNcr->strAcceptmsg = ACCEPTMSG->GetText();

	tinyxml2::XMLElement* STATUSDT = ACCEPTMSG->NextSiblingElement(); // <STATUSDT>20160513153905</STATUSDT>
	pNcr->strStatusdt = STATUSDT->GetText();

	tinyxml2::XMLElement* RESULTMSG = STATUSDT->NextSiblingElement(); // <RESULTMSG>成功</RESULTMSG>
	pNcr->strResultmsg = RESULTMSG->GetText();

	PostThreadMessage(g_HelpThreadID, MSG_NOTIFY_CONTRACT_ROOT, (WPARAM)pNcr, 0);

	return true;
}

bool DoWanrningContractRoot(tinyxml2::XMLElement* RootElment)
{
	tinyxml2::XMLElement* pType = RootElment->FirstChildElement("TYPE");
	if (NULL == pType)
	{
		_tprintf(_T("接收到的xml节点RspType格式错误\n"));
		return false;
	}
	const TCHAR* pcType = pType->GetText();
	_tprintf(_T("%s\n"), pcType);
	if (atoi(pcType) != 2)
	{
		return false;
	}
	tinyxml2::XMLElement* pAccnbr = pType->NextSiblingElement("ACCNBR");
	tinyxml2::XMLElement* pWarningtype = pAccnbr->NextSiblingElement("WARNINGTYPE");
	tinyxml2::XMLElement* pWarningtypemsg = pWarningtype->NextSiblingElement("WARNINGTYPEMSG");
	return true;
}

void doNcResponse(BUFFER_OBJ* bobj)
{
	const TCHAR* pData = _T("<?xml version = \"1.0\" encoding = \"UTF8\"?><ContractRoot><SUCCESS>true</SUCCESS><NOTIFY_CODE>000000</NOTIFY_CODE><RESULTMSG>调用成功</RESULTMSG></ContractRoot>");
	bobj->dwRecvedCount = strlen(pData);
	memcpy_s(bobj->data, bobj->datalen, pData, bobj->dwRecvedCount);
	bobj->SetIoRequestFunction(NC_SendCompFailed, NC_SendCompSuccess);
	if (!PostSend(bobj->pRelatedSObj, bobj))
	{
		CSCloseSocket(bobj->pRelatedSObj);
		freeSObj(bobj->pRelatedSObj);
		freeBObj(bobj);
	}
}