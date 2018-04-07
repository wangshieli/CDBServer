#pragma once

#define SIM_DATA	0X04
typedef enum
{
	SIM_ADD = 0X01,
	SIM_LIST,
}SUBCMD_KH;

bool doSimData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);