#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doSsdqData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doSsdqData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = SSDQ_DATA;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
	{
	case SSDQ_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strSsdq = (pDataObj++)->as<std::string>();

		const TCHAR* pSql = _T("INSERT INTO ssdq_tbl (id,ssdq,xgsj) VALUES(null,'%s',now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strSsdq.c_str());

		if (!ExecuteSql(sql))
		{
			goto error;
		}

		goto success;
	}
	break;

	case SSDQ_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		int nStart = (nTag - 1) * nPage;

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT * FROM llc_tbl ");
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
			var = bobj->pRecorder->GetCollect("llchm");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("llclx");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dxzh");
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