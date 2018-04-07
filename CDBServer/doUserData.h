#pragma once

#define USER_DATA	0X05
typedef enum
{
	USER_ADD = 0X01,
	USER_LOGIN,
	USER_LIST,
}SUBCMD_KH;

bool doUserData(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);