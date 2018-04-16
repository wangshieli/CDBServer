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
	doNcResponse(bobj); // �������ͷ�������
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
//<!--�������� 1������ 2���澯-->
//<TYPE>1< / TYPE>
//<!- ��ˮ��->
//	<GROUP_TRANSACTIONID>1000000252201605131983898593< / GROUP_TRANSACTIONID>
//	<!- ������Ϣ�����磺������ǰ�˴���ȣ��жϱ��β����Ƿ񿢹�Ҳ������ڵ�->
//	<STATUSINFO>����< / STATUSINFO>
//	<!- ����->
//	<ACCNBR>1064915901733< / ACCNBR>
//	<!- ������ʱ��->
//	<SENDDT>20160513153805< / SENDDT>
//	<!--�������ͣ��������������ڵ�6��-->
//	<ACCEPTTYPE>8< / ACCEPTTYPE>
//	<!- ��������->
//	<ACCEPTMSG>����:1064915901733������ͣ������< / ACCEPTMSG>
//	<!- ��Ӧʱ��->
//	<STATUSDT>20160513153905< / STATUSDT>
//	<!- ��Ӧ��Ϣ�����д���Ҳ���������, ���ڵ㲻��Ϊ�жϿ���������->
//	<RESULTMSG>�ɹ�< / RESULTMSG>
//	< / NotifyContractRoot>

bool DoNotifyContractRoot(tinyxml2::XMLElement* root)
{
	NOTIFY_CONTRACT_ROOT* pNcr = new NOTIFY_CONTRACT_ROOT; // ��Ҫ�ͷ�
	tinyxml2::XMLElement* TYPE = root->FirstChildElement(); // <TYPE>1</TYPE> 
	pNcr->strType = TYPE->GetText();

	tinyxml2::XMLElement* GROUP_TRANSACTIONID = TYPE->NextSiblingElement(); // <GROUP_TRANSACTIONID>1000000252201605131983898593</GROUP_TRANSACTIONID>
	pNcr->strGROUP_TRANSACTIONID = GROUP_TRANSACTIONID->GetText();

	tinyxml2::XMLElement* STATUSINFO = GROUP_TRANSACTIONID->NextSiblingElement(); // <STATUSINFO>����</STATUSINFO>
	pNcr->strStatusinfo = STATUSINFO->GetText();

	tinyxml2::XMLElement* ACCNBR = STATUSINFO->NextSiblingElement(); // <ACCNBR>1064915901733</ACCNBR>
	pNcr->strAccnbr = ACCNBR->GetText();

	tinyxml2::XMLElement* SENDDT = ACCNBR->NextSiblingElement(); // <SENDDT>20160513153805</SENDDT>
	pNcr->strSenddt = SENDDT->GetText();

	tinyxml2::XMLElement* ACCEPTTYPE = SENDDT->NextSiblingElement(); // <ACCEPTTYPE>8</ACCEPTTYPE>
	pNcr->strAccepttype = ACCEPTTYPE->GetText();

	tinyxml2::XMLElement* ACCEPTMSG = ACCEPTTYPE->NextSiblingElement(); // <ACCEPTMSG>����:1064915901733������ͣ������</ACCEPTMSG>
	pNcr->strAcceptmsg = ACCEPTMSG->GetText();

	tinyxml2::XMLElement* STATUSDT = ACCEPTMSG->NextSiblingElement(); // <STATUSDT>20160513153905</STATUSDT>
	pNcr->strStatusdt = STATUSDT->GetText();

	tinyxml2::XMLElement* RESULTMSG = STATUSDT->NextSiblingElement(); // <RESULTMSG>�ɹ�</RESULTMSG>
	pNcr->strResultmsg = RESULTMSG->GetText();

	PostThreadMessage(g_HelpThreadID, MSG_NOTIFY_CONTRACT_ROOT, (WPARAM)pNcr, 0);

	int nAccepttype = 0;
	switch (nAccepttype)
	{
	case 1: // ����
	{}
	break;
	case 2: // �˶�
	{}
	break;
	case 3:	// �������޶����
	{}
	break;
	case 4:	// �����س�Ա����
	{}
	break;
	case 5:	// �����س�Աɾ��
	{}
	break;
	case 7:	// �����
	{}
	break;
	case 8:	// ����ͣ������
	{
		// WPARAM 8, PARAM 0      // WPARAM����������ͣ�PARAM ����������
		//if (strcmp(pStatusinfo->GetText(), "����") == 0)
		//{
		//	const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='�û���ͣ' WHERE jrhm='%s'");
		//	TCHAR sql[256];
		//	_stprintf_s(sql, 256, pSql, pAccnbr->GetText());
		//	ExecuteSql(sql);
		//}
		// ��¼������־
	}
	break;
	case 9:	// ͣ�����Ÿ���
	{
		// WPARAM 9, PARAM 0
		//if (strcmp(pStatusinfo->GetText(), "����") == 0)
		//{
		//	const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='����' WHERE jrhm='%s'");
		//	TCHAR sql[256];
		//	_stprintf_s(sql, 256, pSql, pAccnbr->GetText());
		//	ExecuteSql(sql);
		//}
		// ��¼������־
	}
	break;
	case 10: // ����Ԥ������ֵ����
	{}
	break;
	case 11: // ��ӡ��޸ġ�ȡ����������������
	{}
	break;
	case 12: // ����������ָ���������
	{}
	break;
	case 13: // ��ӡ�ȡ����������
	{}
	break;
	case 14: // ʵ���Ʋ�¼
	{}
	break;

	default:
		break;
	}
	return true;
}

bool DoWanrningContractRoot(tinyxml2::XMLElement* RootElment)
{
	tinyxml2::XMLElement* pType = RootElment->FirstChildElement("TYPE");
	if (NULL == pType)
	{
		_tprintf(_T("���յ���xml�ڵ�RspType��ʽ����\n"));
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
	const TCHAR* pData = _T("<?xml version = \"1.0\" encoding = \"UTF8\"?><ContractRoot><SUCCESS>true</SUCCESS><NOTIFY_CODE>000000</NOTIFY_CODE><RESULTMSG>���óɹ�</RESULTMSG></ContractRoot>");
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