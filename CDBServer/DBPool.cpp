#include "stdafx.h"
#include "DBPool.h"
#include <vector>

using std::vector;

#define DB_NAME _T("cardb")
#define DB_USER _T("baolan123")
#define DB_PWD _T("baolan123")
#define DB_DSN _T("mysql_db_a")

#define DB_MIN_CONNER	10
#define DB_MAX_CONNER	50

vector<_ConnectionPtr*> vctConner;
CRITICAL_SECTION csConner;
HANDLE hWaitConner = NULL;

int g_nMinConner;
int g_nMaxConner;
int g_nCurrentNum;

bool CreateConner(_ConnectionPtr& pConner);
bool ReleaseConner(_ConnectionPtr& pConner);
bool ConnectDatabase(_ConnectionPtr& pConner);
void ClearlVector();
bool InitConnerPool(int nMin, int nMax);
_ConnectionPtr* AddNewConner();
_ConnectionPtr* AllocConner();
bool ReInitConner(_ConnectionPtr* pConner);
void FreeConner(_ConnectionPtr* pConner);
bool CreateRecorder(_RecordsetPtr& pRecorder);

bool InitAdoMysql()
{
	::CoInitialize(NULL);
	g_nMinConner = 0;
	g_nMaxConner = 0;
	g_nCurrentNum = 0;
	InitializeCriticalSection(&csConner);
	vctConner.clear();
	hWaitConner = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!InitConnerPool(DB_MIN_CONNER, DB_MAX_CONNER))
	{
		_tprintf(_T("≥ı ºªØ ˝æ›ø‚¡¨Ω” ß∞‹\n"));
		system("pause");
		return false;
	}
	return true;
}

bool InitConnerPool(int nMin, int nMax)
{
	static bool bInitial = true;
	if (bInitial)
	{
		g_nMinConner = nMin;
		g_nMaxConner = nMax;
		for (int i = 0; i < nMin; i++)
		{
			_ConnectionPtr* pConner = new _ConnectionPtr;
			if (!pConner)
				goto error;
			if (!CreateConner(*pConner))
			{
				delete pConner;
				goto error;
			}
			if (!ConnectDatabase(*pConner))
			{
				ReleaseConner(*pConner);
				delete pConner;
				goto error;
			}

			EnterCriticalSection(&csConner);
			vctConner.push_back(pConner);
			++g_nCurrentNum;
			LeaveCriticalSection(&csConner);
		}
		bInitial = false;
	}
	return true;

error:
	ClearlVector();
	return false;
}

bool CreateConner(_ConnectionPtr& pConner)
{
	try
	{
		HRESULT hr = pConner.CreateInstance(__uuidof(Connection));
		if (FAILED(hr))
			return false;
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		return false;
	}
	return true;
}

bool ReleaseConner(_ConnectionPtr& pConner)
{
	if (pConner)
	{
		try
		{
			pConner->Close();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		}

		pConner.Release();
		pConner = NULL;
	}
	return true;
}

bool ConnectDatabase(_ConnectionPtr& pConner)
{
	if (!pConner)
		return false;

	try
	{
		const TCHAR* pConnectionStr = _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s");
		TCHAR pConnection[256];
		memset(pConnection, 0x00, sizeof(pConnection));
		_stprintf_s(pConnection, sizeof(pConnection), pConnectionStr, DB_NAME, DB_DSN, DB_PWD, DB_USER);
		HRESULT hr = pConner->Open(_bstr_t(pConnection), _T(""), _T(""), adModeUnknown);
		if (FAILED(hr))
		{
			return false;
		}
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		return false;
	}

	pConner->CursorLocation = adUseClient;

	return true;
}

void ClearlVector()
{
	for (vector<_ConnectionPtr*>::iterator it = vctConner.begin(); it != vctConner.end(); ++it)
	{
		ReleaseConner(*(*it));
		delete (*it);
	}

	vctConner.clear();
}

_ConnectionPtr* AllocConner()
{
	while (true)
	{
		_ConnectionPtr *pConner = NULL;
		EnterCriticalSection(&csConner);
		if (vctConner.empty())
		{
			if (g_nCurrentNum < g_nMaxConner)
				pConner = AddNewConner();
			else
			{
				ResetEvent(hWaitConner);
				LeaveCriticalSection(&csConner);
				WaitForSingleObject(hWaitConner, INFINITE);
				continue;
			}
		}
		else
		{
			pConner = vctConner.back();
			vctConner.pop_back();
		}
		LeaveCriticalSection(&csConner);
		return pConner;
	}
}

void FreeConner(_ConnectionPtr* pConner)
{
	EnterCriticalSection(&csConner);
	vctConner.push_back(pConner);
	LeaveCriticalSection(&csConner);
	SetEvent(hWaitConner);
}

_ConnectionPtr* AddNewConner()
{
	_ConnectionPtr* pConner = new _ConnectionPtr;
	if (!pConner)
		return NULL;

	if (!CreateConner(*pConner))
	{
		delete pConner;
		return NULL;
	}

	if (!ConnectDatabase(*pConner))
	{
		ReleaseConner(*pConner);
		delete pConner;
		return NULL;
	}

	EnterCriticalSection(&csConner);
	++g_nCurrentNum;
	LeaveCriticalSection(&csConner);

	return pConner;
}

bool Select_From_Tbl(const TCHAR* pSql, _RecordsetPtr& pRecorder)
{
	_ConnectionPtr* pConner = AllocConner();
	if (!pConner)
		return false;

	if (!(*pConner))
	{
		if (!ReInitConner(pConner))
			return false;
	}

	if ((*pConner)->State != adStateOpen)
	{
		ReleaseConner(*pConner);
		if (!ReInitConner(pConner))
			return false;
	}

	if (!CreateRecorder(pRecorder))
	{
		FreeConner(pConner);
		return false;
	}

	try
	{
		HRESULT hr = pRecorder->Open(_bstr_t(pSql), (*pConner).GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);
		if (FAILED(hr))
		{
			goto error;
		}
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		goto error;
	}

	FreeConner(pConner);

	if (pRecorder->adoEOF)
	{
		ReleaseRecorder(pRecorder);
		return false;
	}

	return true;

error:
	ReleaseRecorder(pRecorder);
	FreeConner(pConner);
	return false;
}

bool ReInitConner(_ConnectionPtr* pConner)
{
	if (!CreateConner(*pConner))
		goto error;

	if (!ConnectDatabase(*pConner))
	{
		ReleaseConner(*pConner);
		goto error;
	}

	return true;

error:
	delete pConner;
	EnterCriticalSection(&csConner);
	--g_nCurrentNum;
	LeaveCriticalSection(&csConner);
	return false;
}

bool CreateRecorder(_RecordsetPtr& pRecorder)
{
	try
	{
		HRESULT hr = pRecorder.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr))
			return false;
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		return false;
	}
	return true;
}

void ReleaseRecorder(_RecordsetPtr& pRecorder)
{
	if (pRecorder)
	{
		try
		{
			pRecorder->Close();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		}

		pRecorder.Release();
		pRecorder = NULL;
	}
}

bool ExecuteSql(const TCHAR* pSql)
{
	_ConnectionPtr* pConner = AllocConner();
	if (!pConner)
		return false;

	if (!(*pConner))
	{
		if (!ReInitConner(pConner))
			return false;
	}

	if ((*pConner)->State != adStateOpen)
	{
		ReleaseConner(*pConner);
		if (!ReInitConner(pConner))
			return false;
	}

	try
	{
		VARIANT nRecordAffected = { 0 };
		HRESULT hr = (*pConner)->Execute(_bstr_t(pSql), &nRecordAffected, adCmdText);
		if (FAILED(hr))
			goto error;
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:¥ÌŒÛ–≈œ¢:%s ¥ÌŒÛ¥˙¬Î:%08lx ¥ÌŒÛ‘¥:%s ¥ÌŒÛ√Ë ˆ:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		goto error;
	}

	FreeConner(pConner);
	return true;

error:
	FreeConner(pConner);
	return false;
}

_ConnectionPtr* AllocTransConner()
{
	_ConnectionPtr* pConner = AllocConner();
	if (!pConner)
		return NULL;

	if (!(*pConner))
	{
		if (!ReInitConner(pConner))
			return NULL;
	}

	if ((*pConner)->State != adStateOpen)
	{
		ReleaseConner(*pConner);
		if (!ReInitConner(pConner))
			return NULL;
	}

	return pConner;
}


typedef std::vector<MYSQL* > T_MYSQL_LINK;
T_MYSQL_LINK vctLink;
CRITICAL_SECTION cs;
HANDLE hWaitConnection = NULL;
int g_nMinConnection = 0;
int g_nMaxConnection = 0;
int g_nCurrentCount = 0;

bool Mysql_ConnetDB(MYSQL** pMysql)
{
	*pMysql = mysql_init((MYSQL*)NULL);
	if (NULL == *pMysql)
		return false;

	UINT ml_outime = 5;
	mysql_options(*pMysql, MYSQL_OPT_CONNECT_TIMEOUT, &ml_outime);

	if (NULL == mysql_real_connect(*pMysql, _T("localhost"), DB_USER, DB_PWD, DB_NAME, 0, NULL, 0))
	{
		return false;
	}

	return true;
}

void Mysql_CloseConnection(MYSQL** pMysql)
{
	if (NULL != *pMysql)
	{
		mysql_close(*pMysql);
		*pMysql = NULL;
	}
}

bool Mysql_InitConnectionPool(int nMin, int nMax)
{
	static bool bInitial = true;
	if (bInitial)
	{
		InitializeCriticalSection(&cs);
		hWaitConnection = CreateEvent(NULL, TRUE, FALSE, NULL);
		g_nMinConnection = nMin;
		g_nMaxConnection = nMax;
		for (int i = 0; i < nMin; i++)
		{
			MYSQL* pMysql = NULL;
			if (!Mysql_ConnetDB(&pMysql))
			{
				Mysql_CloseConnection(&pMysql);
				continue;
			}
			EnterCriticalSection(&cs);
			vctLink.push_back(pMysql);
			++g_nCurrentCount;
			LeaveCriticalSection(&cs);
		}
		bInitial = false;
	}
	return !bInitial;
}

void Mysql_RemoveFromPool(MYSQL** pMysql)
{
	if (NULL != *pMysql)
	{
		mysql_close(*pMysql);
		*pMysql = NULL;
	}

	EnterCriticalSection(&cs);
	--g_nCurrentCount;
	LeaveCriticalSection(&cs);
}

MYSQL* Mysql_AllocConnection()
{
	while (true)
	{
		MYSQL* pMysql = NULL;
		EnterCriticalSection(&cs);
		if (vctLink.empty())
		{
			if (g_nCurrentCount < g_nMaxConnection)
			{
				if (!Mysql_ConnetDB(&pMysql))
				{
					LeaveCriticalSection(&cs);
					Mysql_CloseConnection(&pMysql);
					return pMysql;
				}
				else
				{
					++g_nCurrentCount;
				}
			}
			else
			{
				ResetEvent(hWaitConnection);
				LeaveCriticalSection(&cs);
				WaitForSingleObject(hWaitConnection, INFINITE);
				continue;
			}
		}
		else
		{
			pMysql = vctLink.back();
			vctLink.pop_back();
		}
		LeaveCriticalSection(&cs);

		if (0 != mysql_ping(pMysql))
		{
			Mysql_CloseConnection(&pMysql);
			if (!Mysql_ConnetDB(&pMysql))
			{
				Mysql_RemoveFromPool(&pMysql);
			}
		}

		return pMysql;
	}
}

void Mysql_BackToPool(MYSQL* pMysql)
{
	EnterCriticalSection(&cs);
	vctLink.push_back(pMysql);
	LeaveCriticalSection(&cs);
	SetEvent(hWaitConnection);
}

void test()
{
	MYSQL_RES* res = NULL;
	int nStart = 0;
	if (NULL == res)
	{
		MYSQL* pMysql = Mysql_AllocConnection();
		if (NULL == pMysql)
		{
			//  ˝æ›ø‚¥ÌŒÛ
		}

		const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl");
		size_t len = _tcslen(pSql);
		if (0 != mysql_real_query(pMysql, pSql, (ULONG)len))
		{
			// ≤È—Ø¥ÌŒÛ
		}

		my_ulonglong ulCount = mysql_affected_rows(pMysql);

		res = mysql_store_result(pMysql);

		Mysql_BackToPool(pMysql);
	}

	MYSQL_ROW row;
	mysql_data_seek(res, nStart);
	while (row = mysql_fetch_row(res))
	{
		_tprintf(_T("%s : %s : %s\n"), row[0], row[1], row[2]);
	}
}