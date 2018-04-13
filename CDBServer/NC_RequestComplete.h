#pragma once

void NC_AcceptCompFailed(void* _lobj, void* _c_obj);
void NC_AcceptCompSuccess(DWORD dwTranstion, void* _lobj, void* _c_bobj);

void NC_RecvZeroCompFailed(void* _sobj, void* _bobj);
void NC_RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void NC_RecvCompFailed(void* _sobj, void* _bobj);
void NC_RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void NC_SendCompFailed(void* _sobj, void* _bobj);
void NC_SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);