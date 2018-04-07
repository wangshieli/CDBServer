#pragma once
#include <tbb\concurrent_hash_map.h>

using namespace tbb;

typedef struct _tbl_user TBL_USER;
typedef struct _tbl_sim TBL_SIM;
typedef struct _tbl_kh TBL_KH;
typedef struct _tbl_khjl TBL_KHJL;

typedef concurrent_hash_map<unsigned int, TBL_USER*> TYPE_MAP_USER;
typedef concurrent_hash_map<unsigned int, TBL_SIM*>	TYPE_MAP_SIM;
typedef concurrent_hash_map<unsigned int, TBL_KH*>	TYPE_MAP_KH;
typedef concurrent_hash_map<unsigned int, TBL_KHJL*> TYPE_MAP_KHJL;

extern TYPE_MAP_USER map_user;
extern TYPE_MAP_SIM map_sim;
extern TYPE_MAP_KH map_kh;
extern TYPE_MAP_KHJL map_khjl;

void InitDatabase_map();
bool Select_From_Sim();
bool Select_From_Kh();
bool Select_From_Khjl();