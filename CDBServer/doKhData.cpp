#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doKhData.h"
#include "DBPool.h"
#include "DealHeadTail.h"
#include "doDataParser.h"

bool doKhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case KH_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		std::string strLxfs = (pDataObj++)->as<std::string>();
		std::string strJlxm = (pDataObj++)->as<std::string>();
		std::string strBz = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO kh_tbl (id,khmc,lxfs,jlxm,xgsj,bz) VALUES(null,'%s','%s','%s',now(),'%s')");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strLxfs.c_str(), strJlxm.c_str(), strBz.c_str());

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
;		}

		pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl WHERE khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strKhmc.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);

		ParserKhData(_msgpack, bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	case KH_COUNT:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		const TCHAR* pSql = _T("SELECT a.*,b.jlxm,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
SUM(CASE WHEN zt='在用' THEN 1 ELSE 0 END) AS 'sim_using',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'use_15d',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'use_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'due_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'due_15d' \
FROM sim_tbl WHERE khmc='%s') a LEFT JOIN kh_tbl b ON b.khmc='%s'");
		TCHAR sql[1024];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 1024, pSql, strKhmc.c_str(), strKhmc.c_str());

		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		
		ParserKhSimCount(_msgpack, bobj->pRecorder, strKhmc);

		DealTail(sbuf, bobj);
	}
	break;

	case KH_QUERY:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		
		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE khmc='%s' GROUP BY xsrq");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}
		
		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserXsrqData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;

	case KH_LIST:
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
				pSql = _T("SELECT id,Khmc,Userid,Usertype,Fatherid,Jlxm,Dj,Lxfs,Ssdq FROM kh_tbl WHERE Fatherid=1");
				//_stprintf_s(sql, 256, pSql, USER_ITEM);
			}
			else
			{
				pSql = _T("SELECT id,Khmc,Userid,Usertype,Fatherid,Jlxm,Dj,Lxfs,Ssdq FROM kh_tbl WHERE Fatherid=%u");
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
			ParserKh(_msgpack, row);
			row = mysql_fetch_row(bobj->res);
		}

		DealTail(sbuf, bobj);
	}
	break;

	case KH_SIM_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where khmc='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;

	case KH_SIM_XSRQ:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		std::string strXsrq = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where khmc='%s' and xsrq='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strXsrq.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	case KH_SIM_USING:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE khmc='%s' AND zt='在用'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	case KH_SIM_U15D:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE khmc='%s' AND dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY)");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	case KH_SIM_U1M:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE khmc='%s' AND dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH)");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	case KH_SIM_D1M:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE khmc='%s' AND dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH)");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	case KH_SIM_D15D:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE khmc='%s' AND dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY)");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(sql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}

bool OnTimer()
{
	const TCHAR* pSql = _T("SELECT id,Khmc FROM kh_tbl");;
	TCHAR sql[256];
	memset(sql, 0x00, sizeof(sql));

	MYSQL* pMysql = Mysql_AllocConnection();
	if (NULL == pMysql)
	{
		return false;
	}

	size_t len = _tcslen(sql);
	if (0 != mysql_real_query(pMysql, sql, (ULONG)len))
	{
		return false;
	}

	MYSQL_RES* res = mysql_store_result(pMysql);
	if (NULL == res)
	{
		Mysql_BackToPool(pMysql);
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	while (row)
	{
		pSql = _T("SELECT a.*,b.jlxm,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
SUM(CASE WHEN zt='在用' THEN 1 ELSE 0 END) AS 'sim_using',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'On15',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'On1',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'Du1',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'Du15' \
FROM sim_tbl WHERE khmc='%s') a LEFT JOIN kh_tbl b ON b.khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, sizeof(sql), pSql, row[1], row[1]);
		size_t len = _tcslen(sql);
		if (0 != mysql_real_query(pMysql, sql, (ULONG)len))
		{
			row = mysql_fetch_row(res);
			continue;
		}

		MYSQL_RES* subres = mysql_store_result(pMysql);
		if (NULL == subres)
		{
			row = mysql_fetch_row(res);
			continue;
		}

		unsigned int nId = 0, nOn15 = 0, nOn1 = 0, nDu1 = 0, nDu15 = 0;
		MYSQL_ROW subrow = mysql_fetch_row(subres);
		_stscanf_s(row[0], "%u", &nId);
		_stscanf_s(subrow[2], "%u", &nOn15);
		_stscanf_s(subrow[3], "%u", &nOn1);
		_stscanf_s(subrow[4], "%u", &nDu1);
		_stscanf_s(subrow[5], "%u", &nDu15);
		pSql = _T("UPDATE TABLE kh_tbl set On15=%u,On1=%u,Du1=%u,Du15=%u WHERE id=%u");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, sizeof(sql), pSql, nOn15, nOn1, nDu1, nDu15, nId);
		len = _tcslen(sql);
		if (0 != mysql_real_query(pMysql, sql, (ULONG)len))
		{
			mysql_free_result(subres);
			row = mysql_fetch_row(res);
			continue;
		}
		mysql_free_result(subres);

		row = mysql_fetch_row(res);
	}

	Mysql_BackToPool(pMysql);

	mysql_free_result(res);
	return true;
}