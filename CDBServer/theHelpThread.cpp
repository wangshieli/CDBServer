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
		default:
			break;
		}
	}

	return 0;
}
