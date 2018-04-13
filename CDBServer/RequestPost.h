#pragma once

bool PostAcceptEx(LISTEN_OBJ* lobj, int nIndex);

BOOL PostZeroRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);

BOOL PostRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);

BOOL PostSend(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);

void CSCloseSocket(SOCKET_OBJ* _sobj);