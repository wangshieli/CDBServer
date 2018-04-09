#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doUserData.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doUserData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = USER_DATA;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
	{
	case USER_ADD:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strUserName = (pDataObj++)->as<std::string>();
		std::string strUserPwd = (pDataObj++)->as<std::string>();
		int nAuthority = (pDataObj++)->as<int>();
		double dDj = (pDataObj++)->as<double>();

		const TCHAR* pSql = _T("INSERT INTO user_tbl (id,username,password,authority,dj,xgsj) VALUES(null,'%s','%s',%d,%lf,now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strUserName.c_str(), strUserPwd.c_str(), nAuthority, dDj);

		if (!ExecuteSql(sql))
		{
			goto error;
		}
		pSql = _T("SELECT id,authority,dj FROM user_tbl where username='%s' and password='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strUserName.c_str(), strUserPwd.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			goto error;
		}

		_msgpack.pack_array(4);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		_msgpack.pack_array(3);
		_variant_t var = bobj->pRecorder->GetCollect("id");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("authority");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("dj");
		PackCollectDate(_msgpack, var);
		
		DealTail(sbuf, bobj);
	}
	break;

	case USER_LOGIN:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		std::string strUserName = (pDataObj++)->as<std::string>();
		std::string strUserPwd = (pDataObj++)->as<std::string>();
		const TCHAR* pSql = _T("SELECT id,authority,dj FROM user_tbl WHERE username='%s' AND password='%s'");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strUserName.c_str(), strUserPwd.c_str());
		if (!Select_From_Tbl(sql, bobj->pRecorder))
		{
			goto error;
		}

		_msgpack.pack_array(4);
		_msgpack.pack(nCmd);
		_msgpack.pack(nSubCmd);
		_msgpack.pack(0);
		_msgpack.pack_array(1);
		_msgpack.pack_array(3);
		_variant_t var = bobj->pRecorder->GetCollect("id");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("authority");
		PackCollectDate(_msgpack, var);
		var = bobj->pRecorder->GetCollect("dj");
		PackCollectDate(_msgpack, var);

		DealTail(sbuf, bobj);
	}
	break;

	case USER_LIST:
	{
		int nTag = (pObj++)->as<int>();
		int nPage = (pObj++)->as<int>();
		int nStart = (nTag - 1) * nPage;

		if (!bobj->pRecorder)
		{
			const TCHAR* pSql = _T("SELECT id,username,authority,dj,xgsj FROM user_tbl");
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
			_msgpack.pack_array(5);
			var = bobj->pRecorder->GetCollect("id");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("username");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("authority");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("dj");
			PackCollectDate(_msgpack, var);
			var = bobj->pRecorder->GetCollect("xgsj");
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