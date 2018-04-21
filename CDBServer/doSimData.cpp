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
		std::string strIccid = (pDataObj++)->as<std::string>();
		std::string strJlxm = (pDataObj++)->as<std::string>();
		std::string strLltc = (pDataObj++)->as<std::string>();
		std::string strDxzh = (pDataObj++)->as<std::string>();
		std::string strJhrq = (pDataObj++)->as<std::string>();
		std::string strZt = (pDataObj++)->as<std::string>();
		std::string strSsdq = (pDataObj++)->as<std::string>();
		std::string stKhmc = (pDataObj++)->as<std::string>();
		std::string strLlchm = (pDataObj++)->as<std::string>();
		std::string strXsrq = (pDataObj++)->as<std::string>();
		std::string strDqrq = (pDataObj++)->as<std::string>();
		std::string strXfrq = (pDataObj++)->as<std::string>();
		std::string strZxrq = (pDataObj++)->as<std::string>();
		std::string strBz = (pDataObj++)->as<std::string>();
		double dj = (pDataObj++)->as<double>();

		const TCHAR* pSql = _T("INSERT INTO sim_tbl (id,jrhm,iccid,dxzh,llchm,jlxm,lltc,zt,ssdq,khmc,xsrq,bz,dj) values(null,'%s','%s','%s','%s',\
if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),if(STRCMP('%s',''),'%s',null),%lf) \
ON DUPLICATE KEY UPDATE jlxm='%s',lltc='%s',zt='%s',ssdq='%s',khmc='%s',xsrq='%s',bz='%s',dj=%lf");

//		const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llchm,jlxm,khmc,xsrq,zt,lltc,ssdq,dj) \
//value(null,'%s','%s','%s','%s')\
// ON DUPLICATE KEY UPDATE jlxm='%s',lltc='%s'',zt='%s',ssdq='%s',khmc='%s',xsrq='%s',bz='%s',dj=%lf");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strJrhm.c_str(), strIccid.c_str(), strDxzh.c_str(), strLlchm.c_str(),
			strJlxm.c_str(), strJlxm.c_str(), strLltc.c_str(), strLltc.c_str(), strZt.c_str(), strZt.c_str(), strSsdq.c_str(), strSsdq.c_str(), stKhmc.c_str(), stKhmc.c_str(),
			strXsrq.c_str(), strXsrq.c_str(), strBz.c_str(), strBz.c_str(), dj,
			strJlxm.c_str(), strLltc.c_str(), strZt.c_str(), strSsdq.c_str(), stKhmc.c_str(),
			 strXsrq.c_str(), strBz.c_str(), dj);

		if (!ExecuteSql(sql))
		{
			return ErrorInfo(sbuf, _msgpack, bobj);
		}

		pSql = _T("SELECT id,jrhm,iccid,dxzh,khmc,jlxm,zt,llchm,llclx,dj,xsrq,jhrq,xfrq,dqrq,zxrq,bz FROM sim_tbl WHERE jrhm='%s'");
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