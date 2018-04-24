#include "stdafx.h"
#include <msgpack.hpp>
#include <mysql.h>
#include "SignalData.h"
#include "doDataParser.h"

void PackCollectDate(msgpack::packer<msgpack::sbuffer>& _msgpack, const _variant_t& var, bool bDateTime)
{
	switch (var.vt)
	{
	case VT_BSTR:// 字符串
	case VT_LPSTR:
	case VT_LPWSTR:
		_msgpack.pack((const TCHAR*)(_bstr_t)var);
		break;
	case VT_I1:// 无符号字符
	case VT_UI1:
		_msgpack.pack(var.bVal);
		break;
	case VT_I2:// 短整型
		_msgpack.pack(var.iVal);
		break;
	case VT_UI2:// 无符号短整型
		_msgpack.pack(var.uiVal);
		break;
	case VT_INT:// 整形
		_msgpack.pack(var.intVal);
		break;
	case VT_I4:// 整形
	case VT_I8:// 长整形
		_msgpack.pack(var.lVal);
		break;
	case VT_UINT:// 无符号整形
		_msgpack.pack(var.uintVal);
		break;
	case VT_UI4:// 无符号整形
	case VT_UI8:// 无符号长整形
		_msgpack.pack(var.ulVal);
		break;
	case VT_R4:// 浮点型
		_msgpack.pack(var.fltVal);
		break;
	case VT_R8:// 双精度型
		_msgpack.pack(var.dblVal);
		break;
	case VT_DATE:
	{
		SYSTEMTIME st;
		VariantTimeToSystemTime(var.date, &st);
		TCHAR date[32];
		memset(date, 0x00, sizeof(date));
		if (bDateTime)
			_stprintf_s(date, 32, _T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		else
			_stprintf_s(date, 32, _T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
		_msgpack.pack(date);
	}
	break;
	case VT_BOOL:
		_msgpack.pack(var.boolVal);
		break;
	case VT_DECIMAL: //小数  	
	{
		__int64 val = var.decVal.Lo64;
		val *= (var.decVal.sign == 128) ? -1 : 1;
		_msgpack.pack((int)val);
	}
	break;
	case VT_NULL:
	case VT_EMPTY:
	case VT_UNKNOWN:
		_msgpack.pack("");
		break;
	default:
		break;
	}
}

// row[0] = id;
// row[1] = User;
// row[2] = Password;
// row[3] = Authority;
// row[4] = Usertype;
// row[5] = Fatherid;
// row[6] = Dj;
// row[7] = Xgsj;
void ParserUser(msgpack::packer<msgpack::sbuffer>& _msgpack, MYSQL_ROW& row)
{
	unsigned int nTemp = 0;
	_msgpack.pack_array(8);
	_stscanf_s(row[0], _T("%u"), &nTemp);
	_msgpack.pack(nTemp);
	nTemp = 0;
	_msgpack.pack(row[1]);
	_msgpack.pack(row[2]);
	_msgpack.pack(atoi(row[3]));
	_msgpack.pack(atoi(row[4]));
	_stscanf_s(row[5], _T("%u"), &nTemp);
	_msgpack.pack(nTemp);
	_msgpack.pack(atof(row[6]));
	_msgpack.pack(row[7]);
}

void ParserUserData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(6);
	var = pRecorder->GetCollect("id");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("username");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("password");
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

// id, Khmc, Userid, Usertype, Fatherid, Jlxm, Dj, Lxfs, Ssdq
// row[0] = id;
// row[1] = Khmc;
// row[2] = Userid;
// row[3] = Usertype;
// row[4] = Fatherid;
// row[5] = Jlxm;
// row[6] = Dj;
// row[7] = Lxfs;
// row[8] = Ssdq
void ParserKh(msgpack::packer<msgpack::sbuffer>& _msgpack, MYSQL_ROW& row)
{
	unsigned int nTemp = 0;
	_msgpack.pack_array(8);
	_stscanf_s(row[0], _T("%u"), &nTemp);
	_msgpack.pack(nTemp);
	nTemp = 0;
	_msgpack.pack(row[1]);
	_stscanf_s(row[2], _T("%u"), &nTemp);
	_msgpack.pack(nTemp);
	nTemp = 0;
	_msgpack.pack(nTemp);
	_msgpack.pack(atoi(row[3]));
	_stscanf_s(row[4], _T("%u"), &nTemp);
	_msgpack.pack(nTemp);
	nTemp = 0;
	_msgpack.pack(nTemp);
	_msgpack.pack(row[5]);
	_msgpack.pack(atof(row[6]));
	_msgpack.pack(row[7]);
	_msgpack.pack(row[8]);
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

void ParserXsrqData(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder)
{
	_variant_t var;
	_msgpack.pack_array(2);
	var = pRecorder->GetCollect("xsrq");
	PackCollectDate(_msgpack, var, false);
	var = pRecorder->GetCollect("num");
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

void ParserKhSimCount(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder, std::string& strKhmc)
{
	_variant_t var;
	_msgpack.pack_array(9);
	_msgpack.pack(strKhmc);
	var = pRecorder->GetCollect("jlxm");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("lxfs");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("sim_total");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("sim_using");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("use_1m");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("use_15d");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("due_1m");
	PackCollectDate(_msgpack, var);
	var = pRecorder->GetCollect("due_15d");
	PackCollectDate(_msgpack, var);
}