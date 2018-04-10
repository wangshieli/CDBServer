#pragma once

#define DISABLE_NUMBER_DATA 0X10

typedef enum
{
	DN_DISABLE = 0X01,
	DN_ABLE,
}SUBCMD_DISABLE_NUMBER;

bool doDisabledNumber(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);