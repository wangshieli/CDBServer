#pragma once

void ParserUserData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserSimData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserKhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserKhjlData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserDxzhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserLlcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserLltcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserSsdqData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);