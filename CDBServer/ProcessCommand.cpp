#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "ProcessCommand.h"
#include "DealHeadTail.h"
#include "ExcelLoad.h"

int ProcessCommand(BUFFER_OBJ* bobj)
{
	int nFrameLen = 0;
	if (!(nFrameLen = DealHead(bobj)))
		return false;

	int nCmd = 0;
	try
	{
		msgpack::unpacker unpack_;
		msgpack::object_handle result_;
		unpack_.reserve_buffer(bobj->dwRecvedCount);
		memcpy_s(unpack_.buffer(), bobj->dwRecvedCount, bobj->data + 6, nFrameLen);
		unpack_.buffer_consumed(nFrameLen);
		unpack_.next(result_);
		if (msgpack::type::ARRAY != result_.get().type)
		{
			goto error;
		}
		nCmd = result_.get().via.array.ptr->as<int>();

		switch (nCmd)
		{
		case EXCEL_LOAD:
		{
			doExcelLoad(result_, bobj);
		}
		break;
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

	return true;

error:
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	nCmd = 0xbb;
	int nSubCmd = 0xbb;
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack(nCmd);
	msgPack.pack(nSubCmd);
	msgPack.pack(0);

	DealTail(sbuf, bobj);
	return false;
}