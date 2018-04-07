#include "stdafx.h"
#include "Database_map.h"
#include "DBPool.h"

struct _tbl_user
{

};

struct _tbl_sim
{
	unsigned int		id;
	std::string			jrhm;
	std::string			iccid;
	std::string			dxzh;
	std::string			llchm;
	std::string			khmc;
	std::string			xsrq;
	std::string			jhrq;
	std::string			xfrq;
	std::string			dqrq;
	std::string			zxrq;
	std::string			bz;
};

struct _tbl_kh
{
	unsigned int	id;
	std::string			khmc;
	std::string			lxfs;
	std::string			jlxm;
	TYPE_MAP_SIM sim_map;
};

struct _tbl_khjl
{
	unsigned int	id;
	std::string			jlxm;
	std::string			lxfs;
	TYPE_MAP_KH kh_map;
};

TYPE_MAP_USER map_user;
TYPE_MAP_SIM map_sim;
TYPE_MAP_KH map_kh;
TYPE_MAP_KHJL map_khjl;

void InitDatabase_map()
{
	map_user.clear();
	map_sim.clear();
	map_kh.clear();
	map_khjl.clear();
}

bool Select_From_Sim()
{
	const TCHAR* pSql = _T("SELECT * FROM sim_tbl");
	_RecordsetPtr pRecorder;
	if (!Select_From_Tbl(pSql, pRecorder))
		return false;

	TYPE_MAP_SIM::accessor a_sim;
	VARIANT_BOOL bRet = pRecorder->GetadoEOF();
	while (!bRet)
	{
		TBL_SIM* pSim = new TBL_SIM;
		_variant_t varID = pRecorder->GetCollect("id");
		pSim->id = (unsigned int)varID;
		_variant_t varJrhm = pRecorder->GetCollect("jrhm");
		pSim->jrhm = (const TCHAR*)(_bstr_t)varJrhm;
		std::cout << pSim->jrhm << std::endl;

		map_sim.insert(a_sim, pSim->id);
		a_sim->second = pSim;

		pRecorder->MoveNext();
		bRet = pRecorder->GetadoEOF();
	}

	return true;
}

bool Select_From_Kh()
{
	const TCHAR* pSql = _T("SELECT * FROM kh_tbl");
	_RecordsetPtr pRecorder;
	if (!Select_From_Tbl(pSql, pRecorder))
		return false;

	TYPE_MAP_KH::accessor a_kh;
	TYPE_MAP_SIM::accessor a_sim;
	VARIANT_BOOL bRet = pRecorder->GetadoEOF();
	while (!bRet)
	{
		TBL_KH* pKh = new TBL_KH;
		_variant_t varID = pRecorder->GetCollect("id");
		pKh->id = (unsigned int)varID;
		_variant_t varKhmc = pRecorder->GetCollect("khmc");
		pKh->khmc = (const TCHAR*)(_bstr_t)varKhmc;

		for (TYPE_MAP_SIM::iterator it = map_sim.begin(); it != map_sim.end(); ++it)
		{
			if (it->second->khmc == pKh->khmc)
			{
				pKh->sim_map.insert(a_sim, it->first);
				a_sim->second = it->second;
			}
		}

		map_kh.insert(a_kh, pKh->id);
		a_kh->second = pKh;

		pRecorder->MoveNext();
		bRet = pRecorder->GetadoEOF();
	}

	return true;
}

bool Select_From_Khjl()
{
	const TCHAR* pSql = _T("SELECT * FROM khjl_tbl");
	_RecordsetPtr pRecorder;
	if (!Select_From_Tbl(pSql, pRecorder))
		return false;

	TYPE_MAP_KHJL::accessor khjl;
	TYPE_MAP_KH::accessor a_kh;
	VARIANT_BOOL bRet = pRecorder->GetadoEOF();
	while (!bRet)
	{
		TBL_KHJL* pKhjl = new TBL_KHJL;
		_variant_t varID = pRecorder->GetCollect("id");
		pKhjl->id = (unsigned int)varID;
		_variant_t varJlxm = pRecorder->GetCollect("jlxm");
		pKhjl->jlxm = (const TCHAR*)(_bstr_t)varJlxm;

		for (TYPE_MAP_KH::iterator it = map_kh.begin(); it != map_kh.end(); ++it)
		{
			if (it->second->jlxm == pKhjl->jlxm)
			{
				pKhjl->kh_map.insert(a_kh, it->first);
				a_kh->second = it->second;
			}
		}

		map_khjl.insert(khjl, pKhjl->id);
		khjl->second = pKhjl;

		pRecorder->MoveNext();
		bRet = pRecorder->GetadoEOF();
	}

	return true;
}