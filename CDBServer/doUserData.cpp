#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doUserData.h"
#include "DBPool.h"
#include "DealHeadTail.h"
#include "doDataParser.h"

bool doUserData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case USER_ADD:
	{
		// 用户名，密码，权限，类型，用户id，单价，客户名称，经理姓名，联系方式，所属地区
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strUser = (pDataObj++)->as<std::string>();
		std::string strPassword = (pDataObj++)->as<std::string>();
		int nAuthority = (pDataObj++)->as<int>();
		int nUsertype = (pDataObj++)->as<int>();
		unsigned int nFatherid  = (pDataObj++)->as<unsigned int>();
		double nDj = (pDataObj++)->as<double>();
		std::string strKhmc = (pDataObj++)->as<std::string>();
		//std::string strJlxm = (pDataObj++)->as<std::string>();
		//std::string strLxfs = (pDataObj++)->as<std::string>();
		//std::string strSsdq = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO user_tbl (%s) VALUES(null,'%s','%s',%d,%d,%u,%f,now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, USER_ITEM, strUser.c_str(), strPassword.c_str(), nAuthority, nUsertype, nFatherid, nDj);

		MYSQL* pMysql = Mysql_AllocConnection();
		if (NULL == pMysql)
		{
			error_info(bobj, _T("连接数据库失败"));
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		if (!InsertIntoTbl(sql, pMysql))
		{
			UINT uError = mysql_errno(pMysql);
			if (uError == 1062)
			{
				error_info(bobj, _T("已经存在的用户名"));
			}
			else
			{
				error_info(bobj, _T("数据库异常 ErrorCode = %08x, ErrorMsg = %s"), uError, mysql_error(pMysql));
			}
			Mysql_BackToPool(pMysql);
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		my_ulonglong nIndex = mysql_insert_id(pMysql);

		pSql = _T("UPDATE TABLE kh_tbl SET Userid=%u WHERE khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, sizeof(sql), pSql, (unsigned int)nIndex, strKhmc.c_str());
		if (!UpdateTbl(sql, pMysql, bobj))
		{
			// 删除刚添加的用户
			Mysql_BackToPool(pMysql);
			return false;
		}

		pSql = _T("SELECT %s FROM user_tbl WHERE id=%u");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, sizeof(sql), pSql, USER_ITEM, (unsigned int)nIndex);

		MYSQL_RES* res = NULL;
		if (!SelectFromTbl(sql, pMysql, bobj, &res))
		{
			Mysql_BackToPool(pMysql);
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		Mysql_BackToPool(pMysql);

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		MYSQL_ROW row = mysql_fetch_row(res);
		ParserUser(_msgpack, row);
		mysql_free_result(res);
		
		DealTail(sbuf, bobj);
	}
	break;

	case USER_LOGIN:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strUser = (pDataObj++)->as<std::string>();
		std::string strPassword = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("SELECT %s FROM user_tbl WHERE User='%s' AND Password='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, USER_ITEM, strUser.c_str(), strPassword.c_str());

		MYSQL* pMysql = Mysql_AllocConnection();
		if (NULL == pMysql)
		{
			error_info(bobj, _T("连接数据库失败"));
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		MYSQL_RES* res = NULL;
		if (!SelectFromTbl(sql, pMysql, bobj, &res))
		{
			Mysql_BackToPool(pMysql);
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		Mysql_BackToPool(pMysql);

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		MYSQL_ROW row = mysql_fetch_row(res);
		ParserUser(_msgpack, row);
		mysql_free_result(res);

		DealTail(sbuf, bobj);
	}
	break;

	case USER_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		unsigned int nId = (pDataObj++)->as<unsigned int>();
		int nUsertype = (pDataObj++)->as<int>();

		if (!bobj->res)
		{
			const TCHAR* pSql = NULL;
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			if (nUsertype == 1)
			{
				pSql = _T("SELECT %s FROM user_tbl");
				_stprintf_s(sql, 256, pSql, USER_ITEM);
			}
			else
			{
				pSql = _T("SELECT %s FROM user_tbl WHERE Fatherid=%u");
				_stprintf_s(sql, 256, pSql, USER_ITEM, nId);
			}
			
			MYSQL* pMysql = Mysql_AllocConnection();
			if (NULL == pMysql)
			{
				error_info(bobj, _T("连接数据库失败"));
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}

			if (!SelectFromTbl(sql, pMysql, bobj, &bobj->res))
			{
				Mysql_BackToPool(pMysql);
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}

			Mysql_BackToPool(pMysql);
			
			bobj->nRecSetCount = (int)mysql_num_rows(bobj->res);
		}

		InitMsgpack(_msgpack, bobj->res, bobj, nPage, nTag);
		MYSQL_ROW row = mysql_fetch_row(bobj->res);
		while (row)
		{
			ParserUser(_msgpack, row);
			row = mysql_fetch_row(bobj->res);
		}

		DealTail(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}