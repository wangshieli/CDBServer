#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doCardStatus.h"
#include "doAPIResponse.h"
#include "DBPool.h"

void DoCSData(msgpack::object* pObj, BUFFER_OBJ* bobj, const TCHAR* pData);

bool doCardStatus(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();
	bobj->nSubSubCmd = (pObj++)->as<int>();
	bobj->pfndoApiResponse = doCardStatusResponse;

	switch (bobj->nSubCmd)
	{
	case CS_QUERY_JRHM:
	{
		const TCHAR* pData = "GET /m2m_ec/query.do?method=queryCardStatus&access_number=%s&user_id=%s&passWord=%s&sign=%s\r\n\r\n";
		DoCSData(pObj, bobj, pData);
	}
	break;

	case CS_QUERY_ICCID:
	{
		const TCHAR* pData = "GET /m2m_ec/query.do?method=queryCardStatus&iccid=%s&user_id=%s&passWord=%s&sign=%s\r\n\r\n";
		DoCSData(pObj, bobj, pData);
	}
	break;

	default:
		break;
	}

	return true;
}

void DoCSData(msgpack::object* pObj, BUFFER_OBJ* bobj, const TCHAR* pData)
{
	msgpack::object* pArray = (pObj++)->via.array.ptr;
	msgpack::object* pDataObj = (pArray++)->via.array.ptr;
	bobj->strJrhm = (pDataObj++)->as<std::string>();
	std::string dxzh = (pDataObj++)->as<std::string>();

//	const TCHAR* pSql = _T("SELECT userid,pwd,skey FROM dxzh_tbl WHERE dxzh='%s'");
	const TCHAR* pSql = _T("SELECT userid,pwd,skey FROM dxzh_tbl WHERE dxzh LIKE '%%%s%%'");
	TCHAR sql[256];
	memset(sql, 0x00, 256);
	_stprintf_s(sql, 256, pSql, dxzh.c_str());
	if (!Select_From_Tbl(sql, bobj->pRecorder))
	{
		API_Failed(bobj);
		return;
	}
	_variant_t var_userid = bobj->pRecorder->GetCollect("userid");
	_variant_t var_pwd = bobj->pRecorder->GetCollect("pwd");
	_variant_t var_key = bobj->pRecorder->GetCollect("skey");
	std::string key((const char*)(_bstr_t)var_key);
	std::string method = _T("queryCardStatus");
	WOTEDUtils::EncInterfacePtr ep(__uuidof(DesUtils));
	_variant_t varPwd = ep->strEnc((const char*)(_bstr_t)var_pwd, key.substr(0, 3).c_str(), key.substr(3, 3).c_str(), key.substr(6, 3).c_str());
	_variant_t varSign = ep->strEncSign4(bobj->strJrhm.c_str(), (const char*)(_bstr_t)var_userid, (const char*)(_bstr_t)var_pwd, method.c_str(),
		key.substr(0, 3).c_str(), key.substr(3, 3).c_str(), key.substr(6, 3).c_str());

	_stprintf_s(bobj->data, bobj->datalen, pData, (const char*)(_bstr_t)var_userid, bobj->strJrhm.c_str(), (const char*)(_bstr_t)varPwd, (const char*)(_bstr_t)varSign);
	bobj->dwRecvedCount = strlen(bobj->data);
	doApi(bobj);
}