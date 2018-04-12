#pragma once

int DealHead(BUFFER_OBJ* bobj);

void DealTail(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);

void PackCollectDate(msgpack::packer<msgpack::sbuffer>& _msgpack, const _variant_t& var, bool bDateTime = true);

bool ErrorInfo(msgpack::sbuffer& sBuf, msgpack::packer<msgpack::sbuffer>& _msgpack, BUFFER_OBJ* bobj, int nTag = 0);

void InitMsgpack(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder, BUFFER_OBJ* bobj, int nPage, int nTag);