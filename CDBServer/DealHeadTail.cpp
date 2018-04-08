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
	
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  û�����ݿ�ʼ��־
		goto error;
	
	int nFrameLen;
	nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
		goto error;
	
	byte nSum;
	nSum = pRequest[6 + nFrameLen];// �����
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

	memcpy(bobj->data, pCh, nLen);// �Ż�
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

void PackCollectDate(msgpack::packer<msgpack::sbuffer>& _msgpack, const _variant_t& var, bool bDateTime)
{
	switch (var.vt)
	{
	case VT_BSTR:// �ַ���
	case VT_LPSTR:
	case VT_LPWSTR:
		_msgpack.pack((const TCHAR*)(_bstr_t)var);
		break;
	case VT_I1:// �޷����ַ�
	case VT_UI1:
		_msgpack.pack(var.bVal);
		break;
	case VT_I2:// ������
		_msgpack.pack(var.iVal);
		break;
	case VT_UI2:// �޷��Ŷ�����
		_msgpack.pack(var.uiVal);
		break;
	case VT_INT:// ����
		_msgpack.pack(var.intVal);
		break;
	case VT_I4:// ����
	case VT_I8:// ������
		_msgpack.pack(var.lVal);
		break;
	case VT_UINT:// �޷�������
		_msgpack.pack(var.uintVal);
		break;
	case VT_UI4:// �޷�������
	case VT_UI8:// �޷��ų�����
		_msgpack.pack(var.ulVal);
		break;
	case VT_R4:// ������
		_msgpack.pack(var.fltVal);
		break;
	case VT_R8:// ˫������
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
	case VT_NULL:
	case VT_EMPTY:
	case VT_UNKNOWN:
		_msgpack.pack("");
		break;
	default:
		break;
	}
}