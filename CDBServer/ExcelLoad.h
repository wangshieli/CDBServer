#pragma once

#define EXCEL_LOAD	0X01
typedef enum
{
	EXCEL_SIM = 0X01,
	EXCEL_XSQD,
	EXCEL_ZTXX,
	EXCEL_XFQD,
	EXCEL_TKQD,
	EXCEL_ZXQD,
}SUBCMD_EXCEL;

bool doExcelLoad(msgpack::unpacked& pCmdInfo, BUFFER_OBJ* bobj);