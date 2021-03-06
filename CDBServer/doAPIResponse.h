#pragma once
#ifdef _DEBUG
#import "..\WOTEDUtils\bin\Debug\WOTEDUtils.tlb"
#else
#import "..\WOTEDUtils\bin\Release\WOTEDUtils.tlb"
#endif // _DEBUG
using namespace WOTEDUtils;

//extern WOTEDUtils::EncInterfacePtr ep;

void InsertConn(SOCKET_OBJ* sobj);
void RemoveConn(SOCKET_OBJ* sobj);

bool doApi(BUFFER_OBJ* bobj);

void API_Failed(BUFFER_OBJ* bobj);
void API_Successed(BUFFER_OBJ* bobj);

// 如果返回结果不为NULL 使用完之后需要delete
TCHAR* Utf8ConvertAnsi(const TCHAR* strIn, int inLen);

void DoReturnData(BUFFER_OBJ* bobj);

bool doDisNumberResponse(void* bobj);

bool doCardStatusResponse(void* bobj);

bool doServActiveResponse(void* bobj);

bool doPay2Response(void* bobj);

bool doPoolList01Response(void* bobj);

bool doPoolList02Response(void* bobj);

bool doPoolList03Response(void* bobj);