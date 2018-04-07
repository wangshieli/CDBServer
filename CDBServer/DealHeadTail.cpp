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

bool DealHead(BUFFER_OBJ* bobj)
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
	byte pData[1024 * 10];
	memset(pData, 0, 1024 * 10);
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
		closesocket(sobj->sock);
		freeSObj(sobj);
		freeBObj(bobj);
		return;
	}
}