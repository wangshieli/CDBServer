#pragma once

#define SIM_DATA	0X02
typedef enum
{
	SIM_ADD = 0X01,
	SIM_LIST,
	SIM_JRHM,
}SUBCMD_SIM;

bool doSimData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);