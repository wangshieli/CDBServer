#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doSimData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doSimData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = SIM_DATA;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
	{
	case SIM_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJrhm = (pDataObj++)->as<std::string>();
		std::string strIccid = (pDataObj++)->as<std::string>();
		std::string strDxzh = (pDataObj++)->as<std::string>();
		std::string strLlchm = (pDataObj++)->as<std::string>();
		std::string strLlclx = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO sim_tbl (id,jrhm,iccid,llchm,llclx,dxzh) VALUES(null,'%s','%s','%s','%s','%s') ON DUPLICATE KEY UPDATE iccid='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(), strLlchm.c_str(), strLlclx.c_str(), strDxzh.c_str());

		if (!ExecuteSql(sql))
		{
			goto error;
		}

		goto success;
	}
	break;

	case SIM_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		int nStart = (nTag - 1) * nPage;

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT * FROM sim_tbl ");
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
			var = bobj->pRecorder->GetCollect("llcxl");
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

	case SIM_JRHM:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strJrhm = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("SELECT * FROM sim_tbl where jrhm='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str());
		if (!Select_From_Tbl(pSql, bobj->pRecorder))
		{
			goto error;
		}

		_msgpack.pack_array(4);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		_msgpack.pack_array(14);
		_variant_t var;
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
		var = bobj->pRecorder->GetCollect("llcxl");
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