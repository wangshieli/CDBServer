#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doDxzhData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doDxzhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = DXZH_DATA;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
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
			goto error;
		}

		goto success;
	}
	break;

	case DXZH_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		int nStart = (nTag - 1) * nPage;

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT * FROM dxzh_tbl ");
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