#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doDisabledNumber.h"
#include "MemPool.h"
#include "API_RequestComplete.h"
#include "RequestPost.h"
#include "doAPIResponse.h"

bool doDisabledNumber(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	bobj->nSubCmd = (pObj++)->as<int>();
	bobj->pfndoApiResponse = doDisNumberResponse;

	switch (bobj->nSubCmd)
	{
	case DN_DISABLE:
	{
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		bobj->strJrhm = (pDataObj++)->as<std::string>();
		// const TCHAR* pData = "GET /m2m_ec/query.do?method=disabledNumber&user_id=test&access_number=14910000000&acctCd=&passWord=03A2180AB9BF770CE24&sign=03A3E6A47A452C7D7BED1F13FDFDA12D58CA0B8EB944019C810E95C232EE39A8AF7775E24B58A410101A92C7D7BED1F13FDEB944019C810E95C2DE32DEP1A810E9BE&orderTypeId=19\r\n\r\n";
		const TCHAR* pData = "GET /m2m_ec/query.do?method=disabledNumber&user_id=test&access_number=%s&acctCd=&passWord=%s&sign=%s&orderTypeId=19\r\n\r\n";
		_stprintf_s(bobj->data, bobj->datalen, pData, bobj->strJrhm.c_str());
		bobj->dwRecvedCount = strlen(bobj->data);
		doApi(bobj);
	}
	break;

	case DN_ABLE:
	{ 
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		msgpack::object* pDataObj = (pArray++)->via.array.ptr;
		bobj->strJrhm = (pDataObj++)->as<std::string>();
		const TCHAR* pData = "GET /m2m_ec/query.do?method=disabledNumber&user_id=test&access_number=%s&acctCd=&passWord=%s&sign=%s&orderTypeId=20\r\n\r\n";
		_stprintf_s(bobj->data, bobj->datalen, pData, bobj->strJrhm.c_str());
		bobj->dwRecvedCount = strlen(bobj->data);
		doApi(bobj);
	}
	break;
	default:
		break;
	}
	return true;
}