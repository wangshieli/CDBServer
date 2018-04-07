#pragma once

void AcceptCompFailed(void* _lobj, void* _c_obj);
void AcceptCompSuccess(DWORD dwTranstion, void* _lobj, void* _c_bobj);

void RecvZeroCompFailed(void* _sobj, void* _bobj);
void RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void RecvCompFailed(void* _sobj, void* _bobj);
void RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);

void SendCompFailed(void* _sobj, void* _bobj);
void SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj);