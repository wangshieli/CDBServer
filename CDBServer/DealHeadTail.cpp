#include "stdafx.h"
#include <msgpack.hpp>
#include "SignalData.h"
#include "DealHeadTail.h"
#include "RequestComplete.h"
#include "RequestPost.h"
#include "MemPool.h"

byte csum(unsigned char *addr, int count)
{
	byte sum = 0;
	for (int i = 0; i< count; i++)
	{
		sum += (byte)addr[i];
	}
	return sum;
}

int DealHead(BUFFER_OBJ* bobj)
{
	const TCHAR* pRequest = bobj->data;
	int nLen = bobj->dwRecvedCount;
	if (nLen < 8)
		goto error;
	
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  没有数据开始标志
		goto error;
	
	int nFrameLen;
	nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
		goto error;
	
	byte nSum;
	nSum = pRequest[6 + nFrameLen];// 检验和
	if (nSum != csum((unsigned char*)pRequest + 6, nFrameLen))
		goto error;

	if (0x0d != pRequest[nFrameLen + 7])
		goto error;

	return nFrameLen;

error:
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	int nCmd = 0xbb;
	int nSubCmd = 0xbb;
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack(nCmd);
	msgPack.pack(nSubCmd);
	msgPack.pack(0);

	DealTail(sbuf, bobj);
	return 0;
}

void DealTail(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj)
{
	char* pCh = sBuf.data();
	int nLen = sBuf.size();
	byte pData[1024 * 32];
	memset(pData, 0, 1024 * 32);
	memcpy(pData, pCh + 6, nLen - 6);
	byte nSum = csum(pData, nLen - 6);
	sBuf.write("\x00", 1);
	memcpy(pCh + nLen, &nSum, 1);
	sBuf.write("\x0d", 1);
	nLen = sBuf.size();
	nLen -= 8;
	memcpy(pCh + 2, &nLen, 4);
	nLen += 8;

	memcpy(bobj->data, pCh, nLen);// 优化
	bobj->dwRecvedCount = nLen;
	bobj->SetIoRequestFunction(SendCompFailed, SendCompSuccess);
	SOCKET_OBJ* sobj = bobj->pRelatedSObj;
	if (!PostSend(sobj, bobj))
	{
		CSCloseSocket(sobj);
		freeSObj(sobj);
		freeBObj(bobj);
		return;
	}
}

bool ErrorInfo(msgpack::sbuffer& sBuf, msgpack::packer<msgpack::sbuffer>& _msgpack, BUFFER_OBJ* bobj, int nTag/* = 0*/)
{
	if (nTag == 0)
	{
		_msgpack.pack_array(3);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(1);
	}
	else
	{
		_msgpack.pack_array(5);
		_msgpack.pack(bobj->nCmd);
		_msgpack.pack(bobj->nSubCmd);
		_msgpack.pack(nTag);
		_msgpack.pack(1);
		_msgpack.pack(0);
	}

	DealTail(sBuf, bobj);
	return false;
}

void InitMsgpack(msgpack::packer<msgpack::sbuffer>& _msgpack, _RecordsetPtr& pRecorder, BUFFER_OBJ* bobj, int nPage, int nTag)
{
	_msgpack.pack_array(6);
	_msgpack.pack(bobj->nCmd);
	_msgpack.pack(bobj->nSubCmd);
	_msgpack.pack(nTag);
	_msgpack.pack(0);
	_msgpack.pack(bobj->nRecSetCount);

	int nStart = (nTag - 1) * nPage;
	int nTemp = bobj->nRecSetCount - nStart;
	_msgpack.pack_array(nTemp > nPage ? nPage : nTemp);
	bobj->pRecorder->Move(nStart, _variant_t((long)adBookmarkFirst));
}