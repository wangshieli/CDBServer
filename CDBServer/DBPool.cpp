#include "stdafx.h"
#include "SignalData.h"
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



#pragma comment(lib, "libmysql.lib")

//#define DB_NAME _T("cardb")
//#define DB_USER _T("baolan123")
//#define DB_PWD _T("baolan123")
//#define DB_DSN _T("mysql_db_a")

#define MYSQL_32 3307

std::vector<MYSQL*> vctMysql;
CRITICAL_SECTION csMysql;
HANDLE hWaitMysql = NULL;
int g_nMinConnection = 0;
int g_nMaxConnection = 0;
int g_nCurrentNumber = 0;

bool Mysql_ConnectDB(MYSQL** pMysql)
{
	*pMysql = mysql_init((MYSQL*)NULL);
	if (NULL == *pMysql)
		return false;

	if (NULL == mysql_real_connect(*pMysql, _T("localhost"), DB_USER, DB_PWD, DB_NAME, MYSQL_32, NULL, 0))
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
		InitializeCriticalSection(&csMysql);
		vctMysql.clear();
		hWaitMysql = CreateEvent(NULL, TRUE, FALSE, NULL);
		g_nMinConnection = nMin;
		g_nMaxConnection = nMax;
		for (int i = 0; i < nMin; i++)
		{
			MYSQL* pMysql = NULL;
			if (!Mysql_ConnectDB(&pMysql))
			{
				Mysql_CloseConnection(&pMysql);
				continue;
			}
			EnterCriticalSection(&csMysql);
			vctMysql.push_back(pMysql);
			++g_nCurrentNumber;
			LeaveCriticalSection(&csMysql);
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

	EnterCriticalSection(&csMysql);
	--g_nCurrentNumber;
	LeaveCriticalSection(&csMysql);
}

MYSQL* Mysql_AllocConnection()
{
	while (true)
	{
		MYSQL* pMysql = NULL;
		EnterCriticalSection(&csMysql);
		if (vctMysql.empty())
		{
			if (g_nCurrentNumber < g_nMaxConnection)
			{
				if (!Mysql_ConnectDB(&pMysql))
				{
					LeaveCriticalSection(&csMysql);
					Mysql_CloseConnection(&pMysql);
					return pMysql;
				}
				else
				{
					++g_nCurrentNumber;
				}
			}
			else
			{
				ResetEvent(hWaitMysql);
				LeaveCriticalSection(&csMysql);
				WaitForSingleObject(hWaitMysql, INFINITE);
				continue;
			}
		}
		else
		{
			pMysql = vctMysql.back();
			vctMysql.pop_back();
		}
		LeaveCriticalSection(&csMysql);

		if (0 != mysql_ping(pMysql))
		{
			Mysql_CloseConnection(&pMysql);
			if (!Mysql_ConnectDB(&pMysql))
			{
				Mysql_RemoveFromPool(&pMysql);
			}
		}

		return pMysql;
	}
}

void Mysql_BackToPool(MYSQL* pMysql)
{
	EnterCriticalSection(&csMysql);
	vctMysql.push_back(pMysql);
	LeaveCriticalSection(&csMysql);
	SetEvent(hWaitMysql);
}

bool InsertIntoTbl(const TCHAR* sql, MYSQL* pMysql)
{
	size_t len = _tcslen(sql);
	if (0 != mysql_real_query(pMysql, sql, (ULONG)len))
	{
		return false;
	}

	if (mysql_affected_rows(pMysql) == 0)
	{
		return false;
	}

	return true;
}

bool SelectFromTbl(const TCHAR* sql, MYSQL* pMysql, BUFFER_OBJ* bobj, MYSQL_RES** res)
{
	size_t len = _tcslen(sql);
	if (0 != mysql_real_query(pMysql, sql, (ULONG)len))
	{
		error_info(bobj, _T(" ˝æ›ø‚“Ï≥£ ErrorCode = %08x, ErrorMsg = %s"), mysql_errno(pMysql), mysql_error(pMysql));
		return false;
	}

	*res = mysql_store_result(pMysql);
	if (NULL == *res)
	{
		if (mysql_field_count(pMysql) != 0)
		{
			error_info(bobj, _T(" ˝æ›ø‚“Ï≥£ ErrorCode = %08x, ErrorMsg = %s"), mysql_errno(pMysql), mysql_error(pMysql));
		}
		else
		{
			error_info(bobj, _T("Œ¥≤È—ØµΩ∆•≈‰ ˝æ›"));
		}
		return false;
	}

	return true;
}

#define CREATE_USER_TBL _T("CREATE TABLE IF NOT EXISTS user_tbl(id int unsigned not null auto_increment,\
User char(32),\
Password char(41),\
Authority int,Usertype int,\
Fatherid int unsigned,\
Dj double(8,2),\
Xgsj date,\
primary key(id),\
unique key(User))")

bool CreateUserTbl()
{
	MYSQL* pMysql = Mysql_AllocConnection();
	if (NULL == pMysql)
	{
		_tprintf(_T("%s:¡¨Ω”∑˛ŒÒ∆˜ ß∞‹\n"), __FUNCTION__);
		return false;
	}
	size_t len = _tcslen(CREATE_USER_TBL);
	if (0 != mysql_real_query(pMysql, CREATE_USER_TBL, (ULONG)len))
	{
		Mysql_BackToPool(pMysql);
		return false;
	}
	Mysql_BackToPool(pMysql);
	return true;
}

#define CREATE_KH_TBL _T("CREATE TABLE IF NOT EXISTS kh_tbl(id int nusigned not null auto_increment,\
Khmc char(41),\
Userid int unsigned,\
nFatherid int unsigned,\
Jlxm varchar(32),\
Dj double(8,2),\
Lxfs varchar(64),\
Ssdq varchar(64), \
On1m int unsigned,\
On15d int unsigned,\
Du15d int unsigned,\
Du1m int unsigned,\
primary key(id),\
unique key(Khmc))")
#define KH_ITEM _T("id,Khmc,Userid,nFatherid,Jlxm,Dj,Lxfs,Ssdq,On1m,On15d,Du15d,Du1m")
bool CreateKhTbl()
{
	MYSQL* pMysql = Mysql_AllocConnection();
	if (NULL == pMysql)
	{
		_tprintf(_T("%s:¡¨Ω”∑˛ŒÒ∆˜ ß∞‹\n"), __FUNCTION__);
		return false;
	}
	size_t len = _tcslen(CREATE_KH_TBL);
	if (0 != mysql_real_query(pMysql, CREATE_KH_TBL, (ULONG)len))
	{
		Mysql_BackToPool(pMysql);
		return false;
	}
	Mysql_BackToPool(pMysql);
	return true;
}