#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doKhData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doKhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = KH_DATA;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
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
			goto error;
		}

		pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl WHERE khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strKhmc.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			goto error;
		}

		_msgpack.pack_array(4);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);

		_msgpack.pack_array(6);
		_variant_t var;
		var = bobj->pRecorder->GetCollect("id");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("khmc");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("lxfs");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("jlxm");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("xgsj");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("bz");
		PackCollectDate(_msgpack, var);
		bobj->pRecorder->MoveNext();

		DealTail(sbuf, bobj);
	}
	break;

	case KH_COUNT:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		const TCHAR* pSql = _T("SELECT a.*,b.jlxm,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
SUM(CASE WHEN zt='ÔÚÓÃ' THEN 1 ELSE 0 END) AS 'sim_using',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'use_15d',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'use_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'due_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'due_15d' FROM sim_tbl WHERE khmc='%s') a \
LEFT JOIN kh_tbl b ON b.khmc='%s'");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strKhmc.c_str());

		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			goto error;
		}

		_msgpack.pack_array(4);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		_msgpack.pack_array(8);
		_variant_t var;
		var = bobj->pRecorder->GetCollect("jlxm");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("lxfs");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("sim_total");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("sim_using");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("use_1m");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("use_15d");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("due_1m");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("due_15d");
		PackCollectDate(_msgpack, var);

		DealTail(sbuf, bobj);
	}
	break;

	case KH_QUERY:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		int nStart = (nTag - 1) * nPage;

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
				goto error;
			}
		}
		
		int lRstCount = bobj->pRecorder->GetRecordCount();
		_msgpack.pack_array(6);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(nTag);
		_msgpack.pack(0);
		_msgpack.pack(lRstCount);

		int nTemp = lRstCount - nStart;
		_msgpack.pack_array(nTemp > nPage ? nPage : nTemp);
		_variant_t var;
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(2);
			var = bobj->pRecorder->GetCollect("xsrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("num");
			PackCollectDate(_msgpack, var);
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
		int nStart = (nTag - 1) * nPage;

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl ");
			if (!Select_From_Tbl(pSql, bobj->pRecorder))
			{
				goto error;
			}
		}

		int lRstCount = bobj->pRecorder->GetRecordCount();
		_msgpack.pack_array(6);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(nTag);
		_msgpack.pack(0);
		_msgpack.pack(lRstCount);

		int nTemp = lRstCount - nStart;
		_msgpack.pack_array(nTemp > nPage ? nPage : nTemp);
		_variant_t var;
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(6);
			var = bobj->pRecorder->GetCollect("id");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("khmc");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("lxfs");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("jlxm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("xgsj");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("bz");
			PackCollectDate(_msgpack, var);
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
		int nStart = (nTag - 1) * nPage;

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where khmc='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			if (!Select_From_Tbl(pSql, bobj->pRecorder))
			{
				goto error;
			}
		}

		int lRstCount = bobj->pRecorder->GetRecordCount();
		_msgpack.pack_array(6);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(nTag);
		_msgpack.pack(0);
		_msgpack.pack(lRstCount);

		int nTemp = lRstCount - nStart;
		_msgpack.pack_array(nTemp > nPage ? nPage : nTemp);
		_variant_t var;
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(14);
			var = bobj->pRecorder->GetCollect("id");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("jrhm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("iccid");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dxzh");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("khmc");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("llchm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("llclx");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dj");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("xsrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("jhrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("xfrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("dqrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("zxrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("bz");
			PackCollectDate(_msgpack, var);
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
		int nStart = (nTag - 1) * nPage;

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pDataObj++)->as<std::string>();
		std::string strXsrq = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where khmc='%s' and xsrq='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strXsrq.c_str());
			if (!Select_From_Tbl(pSql, bobj->pRecorder))
			{
				goto error;
			}
		}

		int lRstCount = bobj->pRecorder->GetRecordCount();
		_msgpack.pack_array(6);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(nTag);
		_msgpack.pack(0);
		_msgpack.pack(lRstCount);

		int nTemp = lRstCount - nStart;
		_msgpack.pack_array(nTemp > nPage ? nPage : nTemp);
		_variant_t var;
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(14);
			var = bobj->pRecorder->GetCollect("id");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("jrhm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("iccid");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dxzh");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("khmc");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("llchm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("llclx");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dj");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("xsrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("jhrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("xfrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("dqrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("zxrq");
			PackCollectDate(_msgpack, var, false);
			var = bobj->pRecorder->GetCollect("bz");
			PackCollectDate(_msgpack, var);
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

error:
	_msgpack.pack_array(3);
	_msgpack.pack(nCmd);
	_msgpack.pack(nSubCmd);
	_msgpack.pack(1);
	DealTail(sbuf, bobj);
	return false;
}