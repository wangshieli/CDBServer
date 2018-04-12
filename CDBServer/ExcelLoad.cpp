#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "ExcelLoad.h"
#include "DBPool.h"
#include "DealHeadTail.h"

bool doExcelLoad(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	int nCmd = EXCEL_LOAD;
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();
	int nTag = (pObj++)->as<int>();

	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> _msgpack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	switch (nSubCmd)
	{
	case EXCEL_SIM:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		std::string llchm = (pObj++)->as<std::string>();
		std::string llclx = (pObj++)->as<std::string>();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				std::string strIccid = (pDataObj++)->as<std::string>();
				std::string strDxzh = (pDataObj++)->as<std::string>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llchm,llclx) value(null,'%s','%s','%s','%s','%s') ON DUPLICATE KEY UPDATE iccid='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(), strDxzh.c_str(), llchm.c_str(), llclx.c_str(), strIccid.c_str());
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__,
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	case EXCEL_XSQD:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				std::string strKhmc = (pDataObj++)->as<std::string>();
				std::string strXsrq = (pDataObj++)->as<std::string>();
				std::string strXsy = (pDataObj++)->as<std::string>();
				std::string strBz = (pDataObj++)->as<std::string>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("update sim_tbl set khmc='%s',xsrq='%s',xsy='%s',bz='%s' where jrhm='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strXsrq.c_str(), strXsy.c_str(), strBz.c_str(), strJrhm.c_str());
				_variant_t EffectedRecCount;
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__, 
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	case EXCEL_ZTXX:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				std::string strJhrq = (pDataObj++)->as<std::string>();
				std::string strZt = (pDataObj++)->as<std::string>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("update sim_tbl set jhrq='%s',zt='%s' where jrhm='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strJhrq.c_str(), strZt.c_str(), strJrhm.c_str());
				_variant_t EffectedRecCount;
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__, 
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	case EXCEL_XFQD:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				std::string strXfrq = (pDataObj++)->as<std::string>();
				int nMonth = (pDataObj++)->as<int>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("update sim_tbl set dqrq=DATE_ADD(IF('%s'>dqrq,'%s',dqrq),INTERVAL %d MONTH),xfrq='%s' where jrhm='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strXfrq.c_str(), strXfrq.c_str(), nMonth, strXfrq.c_str(), strJrhm.c_str());
				_variant_t EffectedRecCount;
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__, 
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	case EXCEL_TKQD:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("update sim_tbl khmc='',xsrq='',xsy='',bz='' where jrhm='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strJrhm.c_str());
				_variant_t EffectedRecCount;
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__, 
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	case EXCEL_ZXQD:
	{
		_ConnectionPtr* pConner = AllocTransConner();
		if (!pConner)
			goto error;

		(*pConner)->BeginTrans();
		int nCount = (pObj)->via.array.size;
		_variant_t EffectedRecCount;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		try
		{
			for (int i = 0; i < nCount; i++)
			{
				msgpack::object* pDataObj = (pArray++)->via.array.ptr;
				std::string strJrhm = (pDataObj++)->as<std::string>();
				std::string strZxrq = (pDataObj++)->as<std::string>();
				std::string strZt = (pDataObj++)->as<std::string>();
				TCHAR sql[256];
				const TCHAR* pSql = _T("update sim_tbl set zxrq='%s',zt='%s' where jrhm='%s'");
				memset(sql, 0x00, sizeof(sql));
				_stprintf_s(sql, 256, pSql, strZxrq.c_str(), strZt.c_str(), strJrhm.c_str());
				_variant_t EffectedRecCount;
				(*pConner)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			}
			(*pConner)->CommitTrans();
		}
		catch (_com_error e)
		{
			_tprintf(_T("%s-%d:´íÎóÐÅÏ¢:%s ´íÎó´úÂë:%08lx ´íÎóÔ´:%s ´íÎóÃèÊö:%s\n"), __FILE__, __LINE__, 
				e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
			(*pConner)->RollbackTrans();
			FreeConner(pConner);
			goto error;
		}

		FreeConner(pConner);
		goto success;
	}
	break;

	default:
		break;
	}

	return true;

error:
	_msgpack.pack_array(4);
	_msgpack.pack(nCmd);
	_msgpack.pack(nSubCmd);
	_msgpack.pack(nTag);
	_msgpack.pack(1);
	DealTail(sbuf, bobj);
	return false;

success:
	_msgpack.pack_array(4);
	_msgpack.pack(nCmd);
	_msgpack.pack(nSubCmd);
	_msgpack.pack(nTag);
	_msgpack.pack(0);
	DealTail(sbuf, bobj);
	return true;
}