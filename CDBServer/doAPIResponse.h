#pragma once

void InsertConn(SOCKET_OBJ* sobj);
void RemoveConn(SOCKET_OBJ* sobj);

bool doApi(BUFFER_OBJ* bobj);

void API_Failed(BUFFER_OBJ* bobj);
void API_Successed(BUFFER_OBJ* bobj);

// ������ؽ����ΪNULL ʹ����֮����Ҫdelete
TCHAR* Utf8ConvertAnsi(const TCHAR* strIn, int inLen);

void DoReturnData(BUFFER_OBJ* bobj);

bool doDisNumberResponse(void* bobj);