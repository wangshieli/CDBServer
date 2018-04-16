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
			// 主要进行日志记录
			delete pcs;
		}
		break;
		case MSG_CARD_STATUS:
		{
			CARD_STATUS* pcs = (CARD_STATUS*)msg.wParam;
			// 检查目前数据库卡状态，如果不相同就修改
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
			case 1: // 订购
			{}
			break;
			case 2: // 退订
			{}
			break;
			case 3:	// 流量池限额调整
			{}
			break;
			case 4:	// 流量池成员新增
			{}
			break;
			case 5:	// 流量池成员删除
			{}
			break;
			case 7:	// 活卡激活
			{}
			break;
			case 8:	// 办理停机保号
			{
				if (strcmp(pNcr->strStatusinfo.c_str(), "竣工") == 0)
				{
					const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='用户报停' WHERE jrhm='%s'");
					TCHAR sql[256];
					_stprintf_s(sql, 256, pSql, pNcr->strAccnbr.c_str());
					ExecuteSql(sql);
				}
				// 记录竣工日志
			}
			break;
			case 9:	// 停机保号复机
			{
				if (strcmp(pNcr->strStatusinfo.c_str(), "竣工") == 0)
				{
					const TCHAR* pSql = _T("UPDATE sim_tbl SET zt='在用' WHERE jrhm='%s'");
					TCHAR sql[256];
					_stprintf_s(sql, 256, pSql, pNcr->strAccnbr.c_str());
					ExecuteSql(sql);
				}
				// 记录竣工日志
			}
			break;
			case 10: // 流量预警比例值设置
			{}
			break;
			case 11: // 添加、修改、取消卡达量断网功能
			{}
			break;
			case 12: // 达量断网后恢复上网功能
			{}
			break;
			case 13: // 添加、取消单独断网
			{}
			break;
			case 14: // 实名制补录
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
