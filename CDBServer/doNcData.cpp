#include "stdafx.h"
#include "SignalData.h"
#include "doNcData.h"
#include "DBPool.h"
#include "NC_RequestComplete.h"
#include "RequestPost.h"
#include "MemPool.h"
#include "tinyxml2.h"

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

bool DoNotifyContractRoot(tinyxml2::XMLElement* RootElment)
{
	tinyxml2::XMLElement* pType = RootElment->FirstChildElement("TYPE");
	if (NULL == pType)
	{
		_tprintf(_T("���յ���xml�ڵ�RspType��ʽ����\n"));
		return false;
	}
	const TCHAR* pcType = pType->GetText();
	_tprintf(_T("%s\n"), pcType);
	if (atoi(pcType) != 1)
	{
		_tprintf(_T("���յ��ı������ʹ���\n"));
		return false;
	}

	tinyxml2::XMLElement* pStatusinfo = pType->NextSiblingElement("STATUSINFO");
	if (NULL == pStatusinfo)
	{
		return false;
	}
	
	tinyxml2::XMLElement* pAccnbr = pStatusinfo->NextSiblingElement("ACCNBR");
	if (NULL == pAccnbr)
	{
		return false;
	}
	tinyxml2::XMLElement* pAccepttype = pAccnbr->NextSiblingElement("ACCEPTTYPE");
	if (NULL == pAccepttype)
	{
		return false;
	}
	const TCHAR* pcAccepttype = pAccepttype->GetText();
	int nAccepttype = atoi(pcAccepttype);
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
		if (strcmp(pStatusinfo->GetText(), "����") == 0)
		{
			const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='�û���ͣ' WHERE jrhm='%s'");
			TCHAR sql[256];
			_stprintf_s(sql, 256, pSql, pAccnbr->GetText());
			ExecuteSql(sql);
		}
		// ��¼������־
	}
	break;
	case 9:	// ͣ�����Ÿ���
	{}
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