#pragma once

void API_ConnectCompFailed(void* _sobj, void* _bobj);
void API_ConnectCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void API_SendCompFailed(void* _sobj, void* _bobj);
void API_SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void API_RecvZeroCompFailed(void* _sobj, void* _bobj);
void API_RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void API_RecvCompFailed(void* _sobj, void* _bobj);
void API_RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void API_CheckRecvFailed(void* _sobj, void* _bobj);
void API_CheckRecvSuccess(DWORD dwTransion, void* _sobj, void* _bobj);