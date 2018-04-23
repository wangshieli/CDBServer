#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doLlcData.h"
#include "DBPool.h"
#include "DealHeadTail.h"
#include "doDataParser.h"

bool doLlcData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case LLC_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strLlchm = (pDataObj++)->as<std::string>();
		std::string strLlclx= (pDataObj++)->as<std::string>();
		std::string strDxzh = (pDataObj++)->as<std::string>();
		std::string strBz = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO llc_tbl (id,llchm,llclx,dxzh,bz,xgsj) VALUES(null,'%s','%s','%s','%s',now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strLlchm.c_str(), strLlclx.c_str(), strDxzh.c_str(), strBz.c_str());

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		pSql = _T("SELECT id,llchm,llclx,dxzh,bz,xgsj FROM llc_tbl WHERE llchm='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strLlchm.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}
		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);

		ParserLlcData(_msgpack, bobj->pRecorder);

		DealTail(sbuf, bobj);
	}
	break;

	case LLC_LIST:
	{
		bobj->nSubSubCmd = (pObj++)->as<int>();
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,llchm,llclx,dxzh,bz,xgsj FROM llc_tbl ");
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
			ParserLlcData(_msgpack, bobj->pRecorder);
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