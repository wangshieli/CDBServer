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
	KH_SIM_USING,
	KH_SIM_U15D,
	KH_SIM_U1M,
	KH_SIM_D1M,
	KH_SIM_D15D,
}SUBCMD_KH;

bool doKhData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);