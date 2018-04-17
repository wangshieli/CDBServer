#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doPay2.h"
#include "doAPIResponse.h"
#include "DBPool.h"

void DoPay2Data(msgpack::object* pObj, BUFFER_OBJ* bobj, const TCHAR* pData);

bool doPay2(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();
	bobj->pfndoApiResponse = NULL;

	switch (bobj->nSubCmd)
	{
	case 1:
	{
		const TCHAR* pData = _T("GET /m2m_ec/app/pay.do?method=pay2&user_id=%s&passWord=%s&sub_bank_id=%s&pay_money=0.01&order_number=%s&funds_type=1&access_number=%s&sign=%s&callbackURL=%s&callURL=%s\r\n\r\n");
		DoPay2Data(pObj, bobj, pData);
	}
	break;

	case 2:
	{
		const TCHAR* pData = _T("GET /m2m_ec/query.do?method=requestServActive&iccid=%s&user_id=%s&passWord=%s&sign=%s\r\n\r\n");
		DoPay2Data(pObj, bobj, pData);
	}
	break;
	default:
		break;
	}
	return true;
}

void DoPay2Data(msgpack::object* pObj, BUFFER_OBJ* bobj, const TCHAR* pData)
{
	msgpack::object* pArray = (pObj++)->via.array.ptr;
	msgpack::object* pDataObj = (pArray++)->via.array.ptr;
	bobj->strJrhm = (pDataObj++)->as<std::string>();
	std::string dxzh = (pDataObj++)->as<std::string>();

	const TCHAR* pSql = _T("SELECT userid,pwd,skey FROM dxzh_tbl WHERE dxzh='%s'");
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
	std::string method = _T("pay2");

	SYSTEMTIME st;
	GetSystemTime(&st);
	TCHAR order_number[64]; // ¶©µ¥±àºÅ
	memset(order_number, 0x00, sizeof(order_number));
	_stprintf_s(order_number, 64, _T("1025%04d%02d%02d%02d%02d%02d%03d%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, st.wMilliseconds);
	std::string sub_bank_id;
	std::string pay_money;
	std::string funds_type;
	std::string callbackURL;

	WOTEDUtils::EncInterfacePtr ep(__uuidof(DesUtils));
	_variant_t varPwd = ep->strEnc((const char*)(_bstr_t)var_pwd, key.substr(0, 3).c_str(), key.substr(3, 3).c_str(), key.substr(6, 3).c_str());
	_variant_t varSign = ep->strEncSign9(bobj->strJrhm.c_str(), (const char*)(_bstr_t)var_userid, (const char*)(_bstr_t)var_pwd, method.c_str(),
		order_number, sub_bank_id.c_str(), pay_money.c_str(), funds_type.c_str(),
		callbackURL.c_str(), key.substr(0, 3).c_str(), key.substr(3, 3).c_str(), key.substr(6, 3).c_str());

	_stprintf_s(bobj->data, bobj->datalen, pData, (const char*)(_bstr_t)var_userid, bobj->strJrhm.c_str(), (const char*)(_bstr_t)varPwd, (const char*)(_bstr_t)varSign);
	bobj->dwRecvedCount = strlen(bobj->data);
	doApi(bobj);
}