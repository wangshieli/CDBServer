#pragma once
#include <mysql.h>

// 初始化数据库环境
bool InitAdoMysql();

bool Select_From_Tbl(const TCHAR* pSql, _RecordsetPtr& pRecorder);

bool ExecuteSql(const TCHAR* pSql);

void ReleaseRecorder(_RecordsetPtr& pRecorder);

_ConnectionPtr* AllocTransConner();

void FreeConner(_ConnectionPtr* pConner);


#define USER_ITEM	_T("id,User,Password,Authority,Usertype,Fatherid,Dj,Xgsj")

bool Mysql_InitConnectionPool(int nMin, int nMax);

MYSQL* Mysql_AllocConnection();

void Mysql_BackToPool(MYSQL* pMysql);

bool CreateUserTbl();

bool InsertIntoTbl(const TCHAR* sql, MYSQL* pMysql);

bool SelectFromTbl(const TCHAR* sql, MYSQL* pMysql, BUFFER_OBJ* bobj, MYSQL_RES** res);

bool UpdateTbl(const TCHAR* sql, MYSQL* pMysql, BUFFER_OBJ* bobj);