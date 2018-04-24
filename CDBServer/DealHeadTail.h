#pragma once

int DealHead(BUFFER_OBJ* bobj);

void DealTail(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);

bool ErrorInfo(msgpack::sbuffer& sBuf, msgpack::packer<msgpack::sbuffer>& _msgpack, BUFFER_OBJ* bobj, int nTag = 0);


void InitMsgpack(msgpack::packer<msgpack::sbuffer>& _msgpack, MYSQL_RES* res, BUFFER_OBJ* bobj, int nPage, int nTag);
void InitMsgpack(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder, BUFFER_OBJ* bobj, int nPage, int nTag);