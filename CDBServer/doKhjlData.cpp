#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doKhjlData.h"
#include "DBPool.h"
#include "DealHeadTail.h"
#include "doDataParser.h"

bool doKhjlData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case KHJL_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJlxm = (pDataObj++)->as<std::string>();
		std::string strLxfs = (pDataObj++)->as<std::string>();
		std::string strBz = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO khjl_tbl (id,jlxm,lxfs,xgsj,bz) VALUES(null,'%s','%s',now(),'%s')");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJlxm.c_str(), strLxfs.c_str(), strBz.c_str());

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		pSql = _T("SELECT id,jlxm,lxfs,xgsj,bz FROM khjl_tbl WHERE jlxm='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJlxm.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		ParserKhjlData(_msgpack, bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	case KHJL_KH:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJlxm = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl WHERE jlxm='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJlxm.c_str());
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
			ParserKhData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;

	case KHJL_QUERY:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJlxm = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE jlxm='%s' GROUP BY xsrq");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJlxm.c_str());
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

	case KHJL_LIST:
	{
		bobj->nSubSubCmd = (pObj++)->as<int>();
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jlxm,lxfs,xgsj,bz FROM khjl_tbl");
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
			ParserKhjlData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;

	case KHJL_SIM_XSRQ:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJlxm = (pDataObj++)->as<std::string>();
		std::string strXsrq = (pDataObj++)->as<std::string>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where jlxm='%s' and xsrq='%s'");
			TCHAR sql[256];
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJlxm.c_str(), strXsrq.c_str());
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