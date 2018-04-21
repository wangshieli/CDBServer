#pragma once


extern unsigned int g_HelpThreadID;
unsigned int _stdcall theHeperFunction(LPVOID pVoid);

#define MSG_DIS_NUMBER	WM_USER + 100
typedef struct _dis_number
{
	std::string strJrhm;
	std::string strRsptype;
	std::string strResult;
	std::string strResultMsg;
	std::string strGROUP_TRANSACTIONID;
}DIS_NUMBER;


#define MSG_CARD_STATUS	WM_USER + 101
typedef struct _card_status
{
	std::string strResult;
	std::string strResultMsg;
	std::string strProductStatusCd;
	std::string strProductStatusName;
	std::string strPservCreateDate;
	std::string strNumber;
	std::string strGROUP_TRANSACTIONID;
}CARD_STATUS;

#define MSG_SERV_ACTIVE	WM_USER + 102
typedef struct _serv_active
{
	std::string strJrhm;
	std::string strResult;
	std::string strSmsg;
	std::string strGROUP_TRANSACTIONID;
}SERV_ACTIVE;

#define MSG_PAY2	WM_USER + 103
typedef struct _pay
{
	std::string strJrhm;
	std::string strStatus;
	std::string strMessage;
	std::string strUrl;
}PAY2;

typedef struct _pl_01
{
	std::string create_data; // <create_date>2016 - 02 - 19< / create_date>
	std::string pool_infoUnit; // <pool_infoUnit>GB</pool_infoUnit>
	std::string state; // <state>‘⁄”√</state>
	std::string pool_info; // <pool_info>10</pool_info>
	std::string exp_date; // <exp_date>3000-01-01</exp_date>
	std::string acc_nbr; // <acc_nbr>50LLC04244</acc_nbr>
	std::string eff_date; // <eff_date>2016-02-19</eff_date>
}PL_01;



#define MSG_NOTIFY_CONTRACT_ROOT WM_USER + 110
typedef struct _notify_contract_root
{
	std::string strType;
	std::string strGROUP_TRANSACTIONID;
	std::string strStatusinfo;
	std::string strAccnbr;
	std::string strSenddt;
	std::string strAccepttype;
	std::string strAcceptmsg;
	std::string strStatusdt;
	std::string strResultmsg;
}NOTIFY_CONTRACT_ROOT;

typedef struct _wanrning_contract_root
{

}WANRNING_CONTRACT_ROOT;