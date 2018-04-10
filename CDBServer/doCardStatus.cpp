#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doCardStatus.h"
#include "MemPool.h"
#include "API_RequestComplete.h"
#include "RequestPost.h"

bool doCardStatus(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = pCmdInfo.get().via.array.ptr;
	++pObj;
	int nSubCmd = (pObj++)->as<int>();

	bobj->nCmd = CARD_STATUS_DATA;
	bobj->nSubCmd = nSubCmd;

	switch (nSubCmd)
	{
	case CS_QUERY_JRHM:
	{
		const TCHAR* pData = "GET /m2m_ec/query.do?method=queryCardStatus&access_number=14910000000&user_id=test&passWord=32C40A3FC633213EF9EF670D337F2000&sign=03A3E6A47A452C7D7BED1F13FDFDA12D58CA0B8EB944019C810E95C2DE32EE39A8AF7775E24B58A410101A92C7D7BED1F13FDEB944019C810E95C2DE32EDEP1A810E9BED1FADFDFDA12D53\r\n\r\n";
		bobj->dwRecvedCount = strlen(pData);
		strcpy_s(bobj->data, bobj->datalen, pData);
		doApi(bobj);
	}
	break;

	case CS_QUERY_ICCID:
	{
		const TCHAR* pData = "GET /m2m_ec/query.do?method=queryCardStatus&iccid=8986031642100394660&user_id=test&passWord=32C40A3FC633213EF9EF670D337F2000&sign=C50329EE72EA6F858ED7694D2D5B43768DA2458D4E0D5673B9175B1E20295CF3645A3A5C8C7FF1263dC40ABFC6332A3EAC943A1E1A8B1A4C4C5CCBC923347033A88D57551AC783A7BE2B324136FD\r\n\r\n";
		bobj->dwRecvedCount = strlen(pData);
		strcpy_s(bobj->data, bobj->datalen, pData);
		doApi(bobj);
	}
	break;

	default:
		break;
	}

	return true;
}