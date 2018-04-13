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
//		const TCHAR* pSql = _T("SELECT a.sim_total, COALESCE(a.sim_using,0) as sim_using,COALESCE(a.use_15d,0) as use_15d,COALESCE(a.use_1m,0) as use_1m,\
//COALESCE(a.due_1m,0) as due_1m,COALESCE(a.due_15d,0) as due_15d,\
//b.jlxm,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
//SUM(CASE WHEN zt='在用' THEN 1 ELSE 0 END) AS 'sim_using',\
//SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'use_15d',\
//SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'use_1m',\
//SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'due_1m',\
//SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'due_15d' \
//FROM sim_tbl WHERE khmc='%s') a LEFT JOIN kh_tbl b ON b.khmc='%s'");
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
		
		ParserKhSimCount(_msgpack, bobj->pRecorder);

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

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl ");
			if (!Select_From_Tbl(pSql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			ParserKhData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
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
	default:
		break;
	}

	return true;
}