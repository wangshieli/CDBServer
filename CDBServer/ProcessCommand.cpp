#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "ProcessCommand.h"
#include "DealHeadTail.h"
#include "doUserData.h"
#include "doSimData.h"
#include "doKhData.h"
#include "doKhjlData.h"
#include "doLlcData.h"
#include "doLltcData.h"
#include "doDxzhData.h"
#include "ExcelLoad.h"
#include "doSsdqData.h"
#include "doDisabledNumber.h"

int ProcessCommand(BUFFER_OBJ* bobj)
{
	//int nFrameLen = 0;
	//if (!(nFrameLen = DealHead(bobj)))
	//	return false;

	try
	{
		msgpack::unpacker unpack_;
		msgpack::object_handle result_;
		unpack_.reserve_buffer(bobj->dwRecvedCount);
		memcpy_s(unpack_.buffer(), bobj->dwRecvedCount, bobj->data + 6, bobj->dwRecvedCount/*nFrameLen*/);
		unpack_.buffer_consumed(bobj->dwRecvedCount/*nFrameLen*/);
		unpack_.next(result_);
		if (msgpack::type::ARRAY != result_.get().type)
		{
			goto error;
		}
		bobj->nCmd = result_.get().via.array.ptr->as<int>();

		switch (bobj->nCmd)
		{
		case USER_DATA:
		{
			doUserData(result_, bobj);
		}
		break;
		case SIM_DATA:
		{
			doSimData(result_, bobj);
		}
		break;
		case KH_DATA:
		{
			doKhData(result_, bobj);
		}
		break;
		case KHJL_DATA:
		{
			doKhjlData(result_, bobj);
		}
		break;
		case LLC_DATA:
		{
			doLlcData(result_, bobj);
		}
		break;
		case LLTC_DATA:
		{
			doLltcData(result_, bobj);
		}
		break;
		case DXZH_DATA:
		{
			doDxzhData(result_, bobj);
		}
		break;
		case EXCEL_LOAD:
		{
			doExcelLoad(result_, bobj);
		}
		break;
		case SSDQ_DATA:
		{
			doSsdqData(result_, bobj);
		}
		break;
		case DISABLE_NUMBER_DATA:
		{
			doDisabledNumber(result_, bobj);
		}
		default:
			break;
		}
	}
	catch (msgpack::type_error e)
	{
		_tprintf(_T("解析数据类型错误: %s\n"), e.what());
		goto error;
	}
	catch (msgpack::unpack_error e)
	{
		_tprintf(_T("解析错误: %s\n"), e.what());
		goto error;
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:错误信息:%s 错误代码:%08lx 错误源:%s 错误描述:%s\n"), __FILE__, __LINE__,
			e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());
		goto error;
	}
	catch (...)
	{
		_tprintf(_T("未知错误\n"));
		goto error;
	}

	return true;

error:
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	bobj->nCmd = 0xbb;
	bobj->nSubCmd = 0xbb;
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack(bobj->nCmd);
	msgPack.pack(bobj->nSubCmd);
	msgPack.pack(0);

	DealTail(sbuf, bobj);
	return false;
}