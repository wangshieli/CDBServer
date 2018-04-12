#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doDxzhData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doDxzhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (bobj->nSubCmd)
	{
	case DXZH_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strDxzh = (pDataObj++)->as<std::string>();
		std::string strUserID = (pDataObj++)->as<std::string>();
		std::string strPwd = (pDataObj++)->as<std::string>();
		std::string strKey = (pDataObj++)->as<std::string>();
		std::string strtBz = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO dxzh_tbl (id,dxzh,userid,pwd,skey,bz,xgsj) VALUES(null,'%s','%s','%s','%s','%s',now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strDxzh.c_str(), strUserID.c_str(), strPwd.c_str(), strKey.c_str(), strtBz.c_str());

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		pSql = _T("SELECT id,dxzh,userid,pwd,skey,bz,xgsj FROM dxzh_tbl WHERE dxzh='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strDxzh.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		_msgpack.pack_array(4);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		_msgpack.pack_array(7);
		_variant_t var;
		var = bobj->pRecorder->GetCollect("id");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("dxzh");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("userid");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("pwd");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("skey");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("xgsj");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("bz");
		PackCollectDate(_msgpack, var);

		DealTail(sbuf, bobj);
	}
	break;

	case DXZH_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,dxzh,userid,pwd,skey,xgsj,bz FROM dxzh_tbl ");
			if (!Select_From_Tbl(pSql, bobj->pRecorder))
			{
				return ErrorInfo(sbuf, _msgpack, bobj, nTag);
			}
			bobj->nRecSetCount = bobj->pRecorder->GetRecordCount();
		}

		InitMsgpack(_msgpack, bobj->pRecorder, bobj, nPage, nTag);
		_variant_t var;
		VARIANT_BOOL bRt = bobj->pRecorder->GetadoEOF();
		while (!bRt && nPage--)
		{
			_msgpack.pack_array(7);
			var = bobj->pRecorder->GetCollect("id");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dxzh");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("userid");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("pwd");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("skey");
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
	default:
		break;
	}

	return true;
}