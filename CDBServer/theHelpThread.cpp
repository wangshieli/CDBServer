#include "stdafx.h"
#include "theHelpThread.h"
#include "DBPool.h"

unsigned int g_HelpThreadID = 0;

unsigned int _stdcall theHeperFunction(LPVOID pVoid)
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		switch (msg.message)
		{
		case MSG_DIS_NUMBER:
		{
			DIS_NUMBER* pcs = (DIS_NUMBER*)msg.wParam;
			// ��Ҫ������־��¼
			delete pcs;
		}
		break;
		case MSG_CARD_STATUS:
		{
			CARD_STATUS* pcs = (CARD_STATUS*)msg.wParam;
			// ���Ŀǰ���ݿ⿨״̬���������ͬ���޸�
			const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='%s' WHERE jrhm='%s'");
			TCHAR sql[256];
			_stprintf_s(sql, 256, pSql, pcs->strProductStatusName.c_str(), pcs->strNumber.c_str());
			ExecuteSql(sql);
			
			delete pcs;
		}
		break;
		case MSG_NOTIFY_CONTRACT_ROOT:
		{
			NOTIFY_CONTRACT_ROOT* pNcr = (NOTIFY_CONTRACT_ROOT*)msg.wParam;
			int nAccepttype = atoi(pNcr->strAccepttype.c_str());
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
				if (strcmp(pNcr->strStatusinfo.c_str(), "����") == 0)
				{
					const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='�û���ͣ' WHERE jrhm='%s'");
					TCHAR sql[256];
					_stprintf_s(sql, 256, pSql, pNcr->strAccnbr.c_str());
					ExecuteSql(sql);
				}
				// ��¼������־
			}
			break;
			case 9:	// ͣ�����Ÿ���
			{
				if (strcmp(pNcr->strStatusinfo.c_str(), "����") == 0)
				{
					const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='����' WHERE jrhm='%s'");
					TCHAR sql[256];
					_stprintf_s(sql, 256, pSql, pNcr->strAccnbr.c_str());
					ExecuteSql(sql);
				}
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
		}
		break;
		default:
			break;
		}
	}

	return 0;
}
