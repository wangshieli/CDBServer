#include "stdafx.h"
#include "SignalData.h"
#include "db_user.h"
#include "DBPool.h"

bool db_user_add(const TCHAR* pSql, BUFFER_OBJ* bobj)
{
	MYSQL* pMysql = Mysql_AllocConnection();
	if (NULL == pMysql)
	{
		error_info(bobj, _T("数据库访问异常"));
		return false;
	}

	size_t len = _tcslen(pSql);
	if (0 != mysql_real_query(pMysql, pSql, (ULONG)len))
	{
		UINT uError = mysql_errno(pMysql);
		if (1062 == uError) // 主键重复
			error_info(bobj, _T("用户名已存在"));
		else
			error_info(bobj, _T("数据库错误: ErrorCode = %08x, ErrorMsg = %s"), uError, mysql_error(pMysql));
		return false;
	}

	my_ulonglong id = mysql_insert_id(pMysql);

	Mysql_BackToPool(pMysql);
	return true;
}

bool db_user_login(const TCHAR* pSql, BUFFER_OBJ* bobj)
{
	MYSQL* pMysql = Mysql_AllocConnection();
	if (NULL == pMysql)
	{
		error_info(bobj, _T("数据库访问异常"));
		return false;
	}

	size_t len = _tcslen(pSql);
	if (0 != mysql_real_query(pMysql, pSql, (ULONG)len))
	{
		return false;
	}

	my_ulonglong afrow = mysql_affected_rows(pMysql);
	if (0 == afrow)
	{
		error_info(bobj, _T("数据库错误: ErrorCode = %08x, ErrorMsg = %s"), mysql_errno(pMysql), mysql_error(pMysql));
		return false;
	}

	return true;
}