#pragma once

#define KHJL_DATA	0X03
typedef enum
{
	KHJL_ADD = 0X01,
	KHJL_KH,
	KHJL_QUERY,
	KHJL_LIST,
}SUBCMD_KH;

bool doKhjlData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);