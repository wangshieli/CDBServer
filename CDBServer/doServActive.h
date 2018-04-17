#pragma once

#define SERV_ACTIVE_DATA 0X13

typedef enum
{
	SA_JRHM = 0X01,
	SA_ICCID,
}SUBCMD_DISABLE_NUMBER;

bool doServActive(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);