#pragma once

		void PackCollectDate(msgpack::packer<msgpack::sbuffer>& _msgpack, const _variant_t& var, bool bDateTime = true);

void ParserUserData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserSimData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserKhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserKhjlData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

		void ParserXsrqData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserDxzhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserLlcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserLltcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

void ParserSsdqData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder);

		void ParserKhSimCount(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder, std::string& strKhmc);