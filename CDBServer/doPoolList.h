#pragma once

#define POOL_LIST_DATA 0X12

typedef enum
{
	PL_GET_LIST = 0X01,
}SUBCMD_POOL_LIST;

bool doGetPoolList(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);