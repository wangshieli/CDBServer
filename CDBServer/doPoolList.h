#pragma once

#define POOL_LIST_DATA 0X12

typedef enum
{
	PL_GET_LIST = 0X01,// 根据用户名查询流量池列表相关属性
	PL_LLC_INFO, // 根据流量池号获取后向流量池成员列表相关属性
	PL_LLC_POOLQRY, // 查询指定后向流量池的流量总使用情况
}SUBCMD_POOL_LIST;

bool doGetPoolList(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);