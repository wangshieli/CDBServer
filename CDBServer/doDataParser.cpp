#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "doDataParser.h"
#include "DealHeadTail.h"

void ParserUserData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(5);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("username");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("authority");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("dj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
}

void ParserSimData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(16);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("jrhm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("iccid");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("dxzh");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("khmc");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("jlxm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("zt");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("llchm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("llclx");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("dj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xsrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("jhrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("xfrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("dqrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("zxrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("bz");
	PackCollectDate(_msgpack, var);
}

void ParserKhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(6);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("khmc");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("lxfs");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("jlxm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("bz");
	PackCollectDate(_msgpack, var);
}

void ParserKhjlData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(5);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("jlxm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("lxfs");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("bz");
	PackCollectDate(_msgpack, var);
}

void ParserDxzhData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(7);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("dxzh");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("userid");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("pwd");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("skey");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("bz");
	PackCollectDate(_msgpack, var);
}

void ParserLlcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(6);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("llchm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("llclx");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("dxzh");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("bz");
	PackCollectDate(_msgpack, var);
}

void ParserLltcData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(4);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("tcmc");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("tcfl");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
}

void ParserSsdqData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(3);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("ssdq");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("xgsj");
	PackCollectDate(_msgpack, var);
}