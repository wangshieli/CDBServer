#include "stdafx.h"
#include "SignalData.h"
#include <random>

HANDLE hCompPort = NULL;
DWORD g_dwPageSize = 0;

#define NP_THE_ONE_INSTANCE _T("Global\\np_The_One_Instance_Event_SimServer")
HANDLE hUniqueInstance = NULL;
bool UniqueInstance()
{
	hUniqueInstance = ::OpenEvent(EVENT_ALL_ACCESS, false, NP_THE_ONE_INSTANCE);
	if (NULL != hUniqueInstance)
		return false;

	hUniqueInstance = ::CreateEvent(NULL, false, false, NP_THE_ONE_INSTANCE);
	if (NULL == hUniqueInstance)
		return false;

	return true;
}


std::random_device randdev;
std::default_random_engine randeg(randdev());
std::uniform_int_distribution<int> urand(0);

int GetRand()
{
	return urand(randeg);
}