#pragma once

int DealHead(BUFFER_OBJ* bobj);

void DealTail(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);

void PackCollectDate(msgpack::packer<msgpack::sbuffer>& _msgpack, const _variant_t& var, bool bDateTime = true);