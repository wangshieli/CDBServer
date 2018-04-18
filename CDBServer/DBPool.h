#pragma once

// 初始化数据库环境
bool InitAdoMysql();

bool Select_From_Tbl(const TCHAR* pSql, _RecordsetPtr& pRecorder);

bool ExecuteSql(const TCHAR* pSql);

void ReleaseRecorder(_RecordsetPtr& pRecorder);

_ConnectionPtr* AllocTransConner();

void FreeConner(_ConnectionPtr* pConner);

MYSQL* Mysql_AllocConnection();

void Mysql_BackToPool(MYSQL* pMysql);