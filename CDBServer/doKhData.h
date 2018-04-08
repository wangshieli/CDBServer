#pragma once

#define KH_DATA	0X03
typedef enum
{
	KH_ADD = 0X01,
	KH_COUNT,
	KH_QUERY,
	KH_LIST,
	KH_SIM_LIST,
	KH_SIM_XSRQ,
}SUBCMD_KH;

bool doKhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);