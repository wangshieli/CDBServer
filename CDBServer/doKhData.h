#pragma once

#define KH_DATA	0X02
typedef enum
{
	KH_ADD = 0X01,
	KH_COUNT,
	KH_QUERY,
	KH_LIST,
}SUBCMD_KH;

bool doKhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);