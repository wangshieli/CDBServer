#pragma once

#define LLC_DATA	0X06
typedef enum
{
	LLC_ADD = 0X01,
	LLC_LIST,
}SUBCMD_KH;

bool doLlcData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);