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

		goto success;
	}
	break;

	case KH_COUNT:
	{
		std::string strKhmc = (pObj++)->as<std::string>();
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
		_variant_t varJlxm = bobj->pRecorder->GetCollect("jlxm");
		_variant_t varLxfs = bobj->pRecorder->GetCollect("lxfs");
		_variant_t varZksl = bobj->pRecorder->GetCollect("sim_total");
		_variant_t varKysl = bobj->pRecorder->GetCollect("sim_using");
		_variant_t varUse1M = bobj->pRecorder->GetCollect("use_1m");
		_variant_t varUse15D = bobj->pRecorder->GetCollect("use_15d");
		_variant_t varDue1M = bobj->pRecorder->GetCollect("due_1m");
		_variant_t varDue15D = bobj->pRecorder->GetCollect("due_15d");

		ReleaseRecorder(bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	case KH_QUERY:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		std::string strKhmc = (pObj++)->as<std::string>();
		int nStart = (nTag - 1) * nPage;
		
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
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(2);
			_variant_t varXsrq = bobj->pRecorder->GetCollect("xsrq");
			_variant_t varNum = bobj->pRecorder->GetCollect("num");
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		if (nPage > 0)
		{
			ReleaseRecorder(bobj->pRecorder);
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
			const TCHAR* pSql = _T("SELECT * FROM kh_tbl ");
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
		bobj->pRecorder->Move(nStart);
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(2);
			_variant_t varId = bobj->pRecorder->GetCollect("id");
			_variant_t varKhmc = bobj->pRecorder->GetCollect("khmc");
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		if (nPage > 0)
		{
			ReleaseRecorder(bobj->pRecorder);
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

success:
	_msgpack.pack_array(3);
	_msgpack.pack(nCmd);
	_msgpack.pack(nSubCmd);
	_msgpack.pack(0);
	DealTail(sbuf, bobj);
	return true;
}