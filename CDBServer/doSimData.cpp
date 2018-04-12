#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doSimData.h"
#include "DBPool.h"
#include "DealHeadTail.h"
#include "doDataParser.h"

bool doSimData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case SIM_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJrhm = (pDataObj++)->as<std::string>();
		std::string strDxzh = (pDataObj++)->as<std::string>();
		std::string stKhmc = (pDataObj++)->as<std::string>();
		std::string strJlxm = (pDataObj++)->as<std::string>();
		std::string strXsrq = (pDataObj++)->as<std::string>();
		std::string strJhrq = (pDataObj++)->as<std::string>();
		std::string strXfrq = (pDataObj++)->as<std::string>();
		std::string strDqrq = (pDataObj++)->as<std::string>();
		std::string strZxrq = (pDataObj++)->as<std::string>();
		std::string strBz = (pDataObj++)->as<std::string>();
		double dj = (pDataObj++)->as<double>();

//		const TCHAR* pSql = _T("INSERT INTO sim_tbl (id,jrhm,iccid,dxzh,khmc,jlxm,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz) \
//VALUES(null,'%s','%s','%s','%s','%s') ON DUPLICATE KEY UPDATE iccid='%s',jrhm='%s',dxzh='%s'");
		const TCHAR* pSql = _T("UPDATE sim_tbl SET khmc='%s',jlxm='%s',dj='%lf',xsrq='%s',jhrq='%s',xfrq='%s',dqrq='%s',zxrq='%s',bz='%s' WHERE jrhm='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, stKhmc.c_str(), strJlxm.c_str(), dj, strXsrq.c_str(), strJhrq.c_str(), strXfrq.c_str(), strDqrq.c_str(), strZxrq.c_str(), strBz.c_str(), strJrhm.c_str());

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE jrhm='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);

		ParserSimData(_msgpack, bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	case SIM_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl");
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
			ParserSimData(_msgpack, bobj->pRecorder);
			bobj->pRecorder->MoveNext();
			bRt = bobj->pRecorder->GetadoEOF();
		}

		DealTail(sbuf, bobj);
	}
	break;

	case SIM_JRHM:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJrhm = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl where jrhm='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		ParserSimData(_msgpack, bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	default:
		break;
	}

	return true;
}