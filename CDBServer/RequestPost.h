#pragma once

bool PostAcceptEx(LISTEN_OBJ* lobj);

BOOL PostZeroRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);

BOOL PostRecv(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);

BOOL PostSend(SOCKET_OBJ* _sobj, BUFFER_OBJ* _bobj);