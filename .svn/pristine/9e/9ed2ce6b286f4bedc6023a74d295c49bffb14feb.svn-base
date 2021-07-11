// Modbus 驱动。设备参数1为0表示ModbusRTU驱动，为1表示ModbusTCP驱动
// 数据块参数1表示站号。站号必须大于等于1

// DRVTAG的nData1是起始地址位（相对于AI的0地址），nData2是结束地址位，nData3是在该块内的起始位数
#include "AutoGroup_ObjDev_snmp.h"
#include <string> // for sprintf
#include "snmp_pp/snmp_pp.h"
#include "pkcomm/pkcomm.h"
#include "pkdata/pkdata.h"
#include "json/json.h"
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
using namespace std;

#define DEV_N_USERDATA_TARGET			0
#define DEV_N_USERDATA_VERSION			1
#define DEV_N_USERDATA_NONREPS			2
#define DEV_N_USERDATA_MAX_REPS			3
#define DEV_N_USERDATA_SECURITYLEVEL	4

#define DEV_P_USERDATA_SNMP				0
#define DEV_P_USERDATA_TARGET			1

#define DEV_SZ_USERDATA_TARGET			0
#define DEV_SZ_USERDATA_CONTEXTNAME		1
#define DEV_SZ_USERDATA_CONTEXTENGINEID	2

#define TIMER_P_USERDATA_TAGGROUP		0
#define PK_TAGDATA_MAXLEN				4096

PKHANDLE g_hPKData = NULL;

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Snmp::socket_startup();  // Initialize socket subsystem
	g_hPKData = pkInit(NULL, NULL, NULL);
	return 0;
}

PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Snmp::socket_cleanup();  // Shut down socket subsystem
	return 0;
}

long ParseSnmpV3Params(char *pszSnmpParamStr, int &retries, int &timeout, int &non_reps, int &max_reps, 
	OctetStr &privPassword,OctetStr &authPassword,OctetStr &securityName, int &securityModel,int &securityLevel,
	OctetStr &contextName,OctetStr &contextEngineID,long &authProtocol,long &privProtocol)
{
	char szParams[1024] = {0};
	strncpy(szParams, pszSnmpParamStr, sizeof(szParams) - 1);
	char *pTmp = NULL;
	char *ptrParam = PKStringHelper::Strtok(szParams, " ", &pTmp); // 	DB4:X1.1,C:1
	while(ptrParam)
	{
		char *ptr;
		if ( strstr(ptrParam,"-r")!= 0) {                 // parse for retries
			ptr =ptrParam; ptr++; ptr++;
			retries = atoi(ptr);
			if (( retries<0)|| (retries>5)) 
				retries=1; 
		}
		if ( strstr(ptrParam, "-t")!=0) {                 // parse for timeout
			ptr =ptrParam; ptr++; ptr++;
			timeout = atoi( ptr);
			if (( timeout < 100)||( timeout>500)) 
				timeout=100;
		}
		if ( strstr(ptrParam,"-n")!=0) {                 // parse for non repeaters
			ptr =ptrParam;ptr++;ptr++;
			non_reps=atoi( ptr);
			if (( non_reps < 0)||( non_reps>10)) non_reps=0;
		}
		if ( strstr(ptrParam,"-m")!=0) {                 // parse for max repetitions 
			ptr =ptrParam;ptr++;ptr++;
			max_reps=atoi( ptr);
			if ( max_reps < 0) max_reps=1;
		}

		if (strstr(ptrParam,"-auth") != 0) {
			ptr = ptrParam; ptr+=5;
			if (PKStringHelper::StriCmp(ptr, "SHA") == 0)
				authProtocol = SNMP_AUTHPROTOCOL_HMACSHA;
			else if (PKStringHelper::StriCmp(ptr, "MD5") == 0)
				authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;
			else if (PKStringHelper::StriCmp(ptr, "NONE") == 0)
				authProtocol = SNMP_AUTHPROTOCOL_NONE;
			else
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "Warning: ignoring unknown auth protocol: ",ptr);
		}
		if ( strstr( ptrParam,"-priv") != 0) {
			ptr = ptrParam; ptr+=5;
			if (PKStringHelper::StriCmp(ptr, "DES") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_DES;
			else if (PKStringHelper::StriCmp(ptr, "3DESEDE") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_3DESEDE;
			else if (PKStringHelper::StriCmp(ptr, "IDEA") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_IDEA;
			else if (PKStringHelper::StriCmp(ptr, "AES128") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_AES128;
			else if (PKStringHelper::StriCmp(ptr, "AES192") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_AES192;
			else if (PKStringHelper::StriCmp(ptr, "AES256") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_AES256;
			else if (PKStringHelper::StriCmp(ptr, "NONE") == 0)
				privProtocol = SNMP_PRIVPROTOCOL_NONE;
			else
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "Warning: ignoring unknown priv protocol: ",ptr);
		}
		if ( strstr( ptrParam,"-sn")!=0) {
			ptr = ptrParam; ptr+=3;
			securityName = ptr;
			continue;
		}
		if ( strstr( ptrParam, "-sl")!=0) {
			ptr = ptrParam; ptr+=3;
			securityLevel = atoi( ptr);
			if (( securityLevel < SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV) ||
				( securityLevel > SNMP_SECURITY_LEVEL_AUTH_PRIV))
				securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
		}
		if ( strstr( ptrParam, "-sm")!=0) {
			ptr = ptrParam; ptr+=3;
			securityModel = atoi( ptr);
			if (( securityModel < SNMP_SECURITY_MODEL_V1) ||
				( securityModel > SNMP_SECURITY_MODEL_USM))
				securityModel = SNMP_SECURITY_MODEL_USM;
		}
		if ( strstr( ptrParam,"-cn")!=0) {
			ptr = ptrParam; ptr+=3;
			contextName = ptr;
			continue;
		}
		if ( strstr( ptrParam,"-ce")!=0) {
			ptr = ptrParam; ptr+=3;
			contextEngineID = OctetStr::from_hex_string(ptr);
		}
		if ( strstr( ptrParam,"-ua")!=0) {
			ptr = ptrParam; ptr+=3;
			authPassword = ptr;
		}
		if ( strstr( ptrParam,"-up")!=0) {
			ptr = ptrParam; ptr+=3;
			privPassword = ptr;
		}

		ptrParam = PKStringHelper::Strtok(NULL, " ", &pTmp);
	}
	return 0;
}

// 设备的参数：szParam1：版本号，1/2/3，szParam2：读写分区(public;public缺省，分别表示读和写需看设备实际情况）
// 参数3：其他可选参数，包括很多参数如snmp v3的参数，参数4：预留
// pDevice->nUserData[0]存放version,
// pDevice->pUserData[0] 存放target指针，
// pTagGroup->pParam1存放Pdu指针
// pDevice->nUserData[1] = non_reps
// pDevice->nUserData[2] = max_reps;
// pDevice->pUserData[1] 存放Snmp 
// pDevice->nUserData[3] = securityLevel;
// strncpy(pDevice->szUserData[0],  contextName.get_printable() ,PK_DESC_MAXLEN-1);
// strncpy(pDevice->szUserData[1],  contextEngineID.get_printable_hex() ,PK_DESC_MAXLEN-1);
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// 获取到所有的tag点。需要在tag点存储块内偏移（位）、长度（位），组包含的tag点对象列表（以便计算）
	u_short nPort=161;                       // default snmp port is 161
	string strIp = pDevice->szConnParam; // IP,i.e.127.0.0.1
	string strPort;
	string strConnParams = pDevice->szConnParam;
	int nPos = strConnParams.find(';');
	if(nPos >= 0 )
	{
		strIp = strConnParams.substr(0, nPos);
		strPort = strConnParams.substr(nPos +1);
	}
	nPos = strIp.find('=');
	if(nPos >= 0)
		strIp = strIp.substr(nPos + 1);

	nPos = strPort.find('=');
	if(nPos >= 0)
	{
		strPort = strPort.substr(nPos + 1);
		nPort = ::atoi(strPort.c_str());
	}

	UdpAddress address(strIp.c_str());      // make a SNMP++ Generic address
	if (!address.valid()) {           // check validity of address
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Invalid Address or snmp DNS Name %s", strIp.c_str());
		return -1;
	}
	string strReadCommunity = "public";
	string strWriteCommunity = "public";
	string strCommunities = pDevice->szParam2; //存放读写的community
	if(strCommunities.length() > 0)
	{
		int nPos = strCommunities.find(';');
		if(nPos >= 0)
		{
			strReadCommunity = strCommunities.substr(0, nPos);
			strWriteCommunity  =strCommunities.substr(nPos + 1);
		}
		else
			strReadCommunity = strCommunities;
	}

	//----------[ create a SNMP++ session ]-----------------------------------
	int status;
	// bind to any port and use IPv6 if needed
	Snmp *pSnmp = new Snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));
	if ( status != SNMP_CLASS_SUCCESS) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"SNMP++ Session Create Fail, ", pSnmp->error_msg(status));
		return -1;
	}
	pDevice->pUserData[DEV_P_USERDATA_SNMP] = pSnmp;

	snmp_version version= version1; // default is v2c
	int nTmpVersion = 1;
	if(pDevice->szParam1 && strlen(pDevice->szParam1) > 0)
	{
		nTmpVersion = ::atoi(pDevice->szParam1);
		if(nTmpVersion == 1)
			version = version1;
		else if (nTmpVersion == 2)
			version = version2c;
		else if(nTmpVersion == 3)
			version = version3;
	}
	pDevice->nUserData[DEV_N_USERDATA_VERSION] = version;

	int retries=1;                          // default retries is 1
	int timeout=100;                        // default is 1 second
	OctetStr communityRead(strReadCommunity.c_str());           // community name
	OctetStr communityWrite(strWriteCommunity.c_str());           // community name
	int non_reps=0;                         // non repeaters default is 0
	int max_reps=500;                       // maximum repetitions default is 10

	//#ifdef _SNMPv3
	OctetStr privPassword("");
	OctetStr authPassword("");
	OctetStr securityName("");
	int securityModel = SNMP_SECURITY_MODEL_USM;
	int securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
	OctetStr contextName("");
	OctetStr contextEngineID("");
	long authProtocol = SNMP_AUTHPROTOCOL_NONE;
	long privProtocol = SNMP_PRIVPROTOCOL_NONE;
	//#endif // snmp v3

	ParseSnmpV3Params(pDevice->szParam3, retries, timeout, non_reps, max_reps, privPassword, authPassword, securityName,
		securityModel, securityLevel, contextName, contextEngineID, authProtocol, privProtocol);
	pDevice->nUserData[DEV_N_USERDATA_NONREPS] = non_reps;
	pDevice->nUserData[DEV_N_USERDATA_MAX_REPS] = max_reps;
	pDevice->nUserData[DEV_N_USERDATA_SECURITYLEVEL] = securityLevel;
	strncpy(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTNAME], contextName.get_printable(), PK_DESC_MAXLEN - 1);
	strncpy(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTENGINEID], contextEngineID.get_printable_hex(), PK_DESC_MAXLEN - 1);
	address.set_port(nPort);

	v3MP *v3_MP;
	SnmpTarget *target;
	//---------[ init SnmpV3 ]--------------------------------------------
	if (version == version3) {
		const char *engineId = "snmpBulk";
		const char *filename = "snmpv3_boot_counter";
		unsigned int snmpEngineBoots = 0;
		int status;
		status = getBootCounter(filename, engineId, snmpEngineBoots);
		if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Error loading snmpEngineBoots counter: %d", status);
			return -1;
		}
		snmpEngineBoots++;
		status = saveBootCounter(filename, engineId, snmpEngineBoots);
		if (status != SNMPv3_OK)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Error saving snmpEngineBoots counter: %d", status);
			return -1;
		}

		int construct_status;
		v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);
		if (construct_status != SNMPv3_MP_OK)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Error initializing v3MP: %d", construct_status);
			return -1;
		}

		USM *usm = v3_MP->get_usm();
		usm->add_usm_user(securityName,
			authProtocol, privProtocol,
			authPassword, privPassword);

		target = new UTarget(address);
		target->set_version( version);          // set the SNMP version SNMPV1 or V2 or V3
		target->set_retry( retries);            // set the number of auto retries
		target->set_timeout( timeout);          // set timeout
		((UTarget *)target)->set_security_model( securityModel);
		((UTarget *)target)->set_security_name( securityName);
	}
	else
	{
		// MUST create a dummy v3MP object if _SNMPv3 is enabled!
		int construct_status;
		v3_MP = new v3MP("dummy", 0, construct_status);

		target = new CTarget(address);
		target->set_version( version);         // set the SNMP version SNMPV1 or V2
		target->set_retry( retries);           // set the number of auto retries
		target->set_timeout( timeout);         // set timeout
		((CTarget *)target)->set_readcommunity(communityRead); // set the read community name
		((CTarget *)target)->set_writecommunity(communityWrite);
	} // if (version == version3)

	if ( !target->valid())
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "SNMP target(%s:%s) version:%d,is invalid", 
			strIp.c_str(), strPort.c_str(),((version==version3) ? (version) : (version+1)));
		return SNMP_CLASS_INVALID_TARGET;
	}

	pDevice->pUserData[DEV_P_USERDATA_TARGET] = target;

	//-------[ issue the request, blocked mode ]-----------------------------
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "SNMP++ GetBulk to %s,snmp version=%d,Retries=%d,Timeout=%d ms,Non Reptrs=%d,Max Reps=%d", 
		strIp.c_str(), ((version==version3) ? (version) : (version+1)), retries,timeout * 10,non_reps, max_reps);
	if (version == version3)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "SNMP v3, securityName=%s, securityLevel=%d, securityModel= %d,contextName= %s,contextEngineID=%s", 
			securityName.get_printable(),securityLevel,securityModel,contextName.get_printable(),contextEngineID.get_printable());
	}

	// 对于snmpV1，不支持分组，每个组就1个元素
	// 对于snmpV2和snmpV3，支持分组，每个组暂定为300个字节（瞎估的）。每个分组只能指定一个communityName（放在地址target中）
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	if(version == version1)
		groupOption.nIsGroupTag = 0; // 不支持Group操作
	groupOption.nMaxBytesOneGroup = 1000; // modbus每种设备都有所不同，应该是作为参数传过来比较合适

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup); // 如果支持Group操作，那么每个group300个字节
	vecTags.clear();

	unsigned int i = 0;
	for(; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGroup = vecTagGroup[i];
		// construct a Pdu object
		Pdu *pPdu = new Pdu();    
		pTagGroup->pParam1 = pPdu;
		if(version == version3)
		{
			pPdu->set_security_level( securityLevel);
			pPdu->set_context_name (contextName);
			pPdu->set_context_engine_id(contextEngineID);
		}

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGroup->nPollRate;
		timerInfo.pUserData[TIMER_P_USERDATA_TAGGROUP] = pTagGroup;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器
	}

	return 0;
}

/*
	反初始化设备
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	int nTimerNum = 0;
	//PKTIMER * pTimers = Drv_GetTimers(pDevice, &nTimerNum);
	//int i = 0;
	//for(i = 0; i < nTimerNum; i ++)
	//{
	//	PKTIMER *pTimerInfo = &pTimers[i];
	//	DRVGROUP *pTagGroup = (DRVGROUP *)pTimerInfo->pUserData[0];
	//	delete pTagGroup;
	//	pTagGroup = NULL;
	//	Drv_DestroyTimer(pDevice, pTimerInfo);
	//}
	return 0;
}

//#define COVERT2TAGDATA(T,NVAL)	{T _temp=NVAL; memcpy(&pTag->szData, &_temp, sizeof(T));}
// 将字符串值写入到tag点的二进制缓冲区中去
int SetTagDataString(PKTAG *pTag, const char *szValue, int nValLen)
{
	if (pTag->nDataType == TAG_DT_BLOB)
		Drv_SetTagData_Binary(pTag, (void *)szValue, nValLen);
	else
		Drv_SetTagData_Text(pTag, szValue);
	//pTag->nTimeSec = pTag->nTimeMilSec = 0;
	//if (pTag->nDataType == TAG_DT_BOOL)
	//	pTag->szData[0] = (::atoi(szValue) == 0 ? 0 : 1);
	//else if (pTag->nDataType == TAG_DT_CHAR)
	//	pTag->szData[0] = szValue[0];
	//else if (pTag->nDataType == TAG_DT_UCHAR)
	//	pTag->szData[0] = (unsigned char)szValue[0];
	//else if (pTag->nDataType == TAG_DT_INT16)
	//	COVERT2TAGDATA(short, ::atoi(szValue))
	//else if (pTag->nDataType == TAG_DT_UINT16)
	//COVERT2TAGDATA(unsigned short, ::atoi(szValue))
	//else if (pTag->nDataType == TAG_DT_INT32)
	//COVERT2TAGDATA(int, ::atoi(szValue))
	//else if (pTag->nDataType == TAG_DT_UINT32)
	//COVERT2TAGDATA(unsigned int, ::atol(szValue))
	//else if (pTag->nDataType == TAG_DT_INT64)
	//COVERT2TAGDATA(int64_t, ::atol(szValue))
	//else if (pTag->nDataType == TAG_DT_UINT64)
	//COVERT2TAGDATA(uint64_t, ::atol(szValue))
	//else if (pTag->nDataType == TAG_DT_FLOAT)
	//COVERT2TAGDATA(float, ::atof(szValue))
	//else if (pTag->nDataType == TAG_DT_DOUBLE)
	//COVERT2TAGDATA(double, ::atof(szValue))
	//else if (pTag->nDataType == TAG_DT_TEXT)
	//strncpy(pTag->szData, szValue, sizeof(pTag->szData) - 1);
	//else if (pTag->nDataType == TAG_DT_BLOB)
	//	memcpy(pTag->szData, szValue, nValLen > pTag->nDataLen ? pTag->nDataLen : nValLen);
	//else
	//	return -1;
	return 0;
}

int SetGroupOidValue(DRVGROUP *pTagGroup, const char *szOid, const char *szValue)
{
	for(int iTag = 0; iTag < pTagGroup->vecTags.size(); iTag ++)
	{
		PKTAG *pTag = pTagGroup->vecTags[iTag];
		if(PKStringHelper::StriCmp(pTag->szAddress, szOid) == 0)
		{
			SetTagDataString(pTag, szValue, strlen(szValue));
			break;
		}
	}
	return 0;
}

int SetGroupOidQuality(DRVGROUP *pTagGroup, const char *szOid, int nQuality)
{
	for(int iTag = 0; iTag < pTagGroup->vecTags.size(); iTag ++)
	{
		PKTAG *pTag = pTagGroup->vecTags[iTag];
		pTag->nTimeSec = pTag->nTimeMilSec;
		if(PKStringHelper::StriCmp(pTag->szAddress, szOid) == 0)
		{
			pTag->nQuality = nQuality;
			break;
		}
	}
	return 0;
}

// tag点地址：1.3.6.1.2.1.1.1.0, 1.3.6.1.2.1.1.1.0
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	// 组织读取消息
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[TIMER_P_USERDATA_TAGGROUP];
	Pdu *pPdu = (Pdu *) pTagGroup->pParam1;

	SnmpTarget *pTarget = (SnmpTarget *)pDevice->pUserData[DEV_P_USERDATA_TARGET];
	Snmp *pSnmp = (Snmp *)pDevice->pUserData[DEV_P_USERDATA_SNMP];

	snmp_version version = (snmp_version)pDevice->nUserData[DEV_N_USERDATA_VERSION];
	int non_reps = pDevice->nUserData[DEV_N_USERDATA_NONREPS];
	int max_reps = pDevice->nUserData[DEV_N_USERDATA_MAX_REPS];

	// 初始化
	pPdu->clear();
	for (int iTag = 0; iTag < pTagGroup->vecTags.size(); iTag++)
	{
		PKTAG *pTag = pTagGroup->vecTags[iTag];
		// 去掉communityName就是OID
		string strOid = pTag->szAddress;
		if (pTagGroup->bIsBulkOid)
			strOid = strOid.substr(5); // 去掉bulk: 5个字母

		Oid oid(strOid.c_str());
		if (!oid.valid()) {            // check validity of user oid
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Invalid Oid,  %s", strOid.c_str());
			continue;
		}

		Vb vb;
		vb.set_oid(oid); // 会重新复制oid
		*pPdu += vb; // 会重新复制vb，故不用new
	}

	if (!pTagGroup->bIsBulkOid)
	{
		int status = pSnmp->get(*pPdu, *pTarget);
		if (status != SNMP_CLASS_SUCCESS)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP GetBulk(group:%s) error,err:%s,tag个数:%d,tagname:%s,tagaddr:%s",
				pTagGroup->szAutoGroupName, pSnmp->error_msg(status), pTagGroup->vecTags.size(), pTagGroup->vecTags[0]->szName, pTagGroup->vecTags[0]->szAddress);
			UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_BAD_UNKNOWN_REASON, "unknown reason");
			return 0;
		}

		int nVbCount = pPdu->get_vb_count();
		for (int z = 0; z < nVbCount; z++)
		{
			Vb vb;
			pPdu->get_vb(vb, z); // 每一个tag点!
			const char *szOid = vb.get_printable_oid();
			if (version3 == version)
			{
				if (pPdu->get_type() == REPORT_MSG) {
					Oid tmp;
					vb.get_oid(tmp);
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "Received a reportPdu:%s,oid:%s,value:%s", pSnmp->error_msg(tmp),
						vb.get_printable_oid(), vb.get_printable_value());
				}
			}

			if ((vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW) ||
				(vb.get_syntax() == sNMP_SYNTAX_NOSUCHINSTANCE) ||
				(vb.get_syntax() == sNMP_SYNTAX_NOSUCHOBJECT))
			{// 发生了错误！
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP get(group:%s) error, syntax error:%d", pTagGroup->szAutoGroupName, vb.get_syntax());
				SetGroupOidQuality(pTagGroup, szOid, TAG_QUALITY_BAD_UNKNOWN_REASON);
			}
			else if (vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW)
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP get(group:%s) ,End of MIB view.", pTagGroup->szAutoGroupName);
			else
			{ // 获取到正确的值
				const char *szValue = vb.get_printable_value();
				SetGroupOidValue(pTagGroup, szOid, szValue);  // 为每个点赋值
			}
		} // for ( int z=0;z<pdu.get_vb_count();z++) 

		int nRet = UpdateGroupData(pDevice, pTagGroup);
		return nRet;
	}

	// walk，获取一批命令,仅仅对1个tag点操作，tag点地址需为：walk:1.3.6.1.2.xxxx
	// version1 version 2 and 3
	PKTAG *pTag = pTagGroup->vecTags[0];
	string strGroupOid = pTag->szAddress;
	strGroupOid = strGroupOid.substr(5); // 去掉前面的walk:
	int status = pSnmp->get_bulk(*pPdu, *pTarget, non_reps, max_reps);
	if(status != SNMP_CLASS_SUCCESS) 
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP GetBulk(group:%s) error,err:%s,tagname:%s,tagaddr:%s",
			pTagGroup->szAutoGroupName, pSnmp->error_msg(status), pTag->szName, pTag->szAddress);
		UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_BAD_UNKNOWN_REASON, "unknown reason");
		return 0;
	}
	
	Vb vb;
	Json::Value jsonVal;
	bool bCycleOver = false;
	while (status == SNMP_CLASS_SUCCESS && !bCycleOver)
	{
		int nVbCount = pPdu->get_vb_count();
		for (int z = 0; z < nVbCount; z++)
		{
			pPdu->get_vb(vb, z); // 每一个tag点!
			const char *szOid = vb.get_printable_oid();
			if (strncmp(szOid, strGroupOid.c_str(), strGroupOid.length()) != 0) // 这个层次的Oid已经循环完毕了
			{
				bCycleOver = true;
				break;
			}

			if (version3 == version)
			{
				if (pPdu->get_type() == REPORT_MSG) {
					Oid tmp;
					vb.get_oid(tmp);
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "Received a reportPdu:%s,oid:%s,value:%s", pSnmp->error_msg(tmp),
						vb.get_printable_oid(), vb.get_printable_value());
				}
			}

			if ((vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW) ||
				(vb.get_syntax() == sNMP_SYNTAX_NOSUCHINSTANCE) ||
				(vb.get_syntax() == sNMP_SYNTAX_NOSUCHOBJECT))
			{// 发生了错误！
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP get(group:%s) error, syntax error:%d", pTagGroup->szAutoGroupName, vb.get_syntax());
				SetGroupOidQuality(pTagGroup, szOid, TAG_QUALITY_BAD_UNKNOWN_REASON);
			}
			else if (vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW)
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP get(group:%s) ,End of MIB view.", pTagGroup->szAutoGroupName);
			else
			{ // 获取到正确的值
				const char *szValue = vb.get_printable_value();
				if (pTagGroup->bIsBulkOid) // 是批量获取数据，因此需要
				{
					Json::Value jsonOne;
					string strIndex = szOid;
					if (strIndex.length() > strGroupOid.length())
						strIndex = strIndex.substr(strGroupOid.length() + 1);
					else
						strIndex = "";
					jsonOne["index"] = strIndex.c_str();
					jsonOne["value"] = szValue;
					jsonVal.append(jsonOne);
				}
			}
		} // for ( int z=0;z<pdu.get_vb_count();z++) 

		// 如果是读取一个块，那么继续轮询下一个oid
		// last vb becomes seed of next rquest
		pPdu->set_vblist(&vb, 1);

		status = pSnmp->get_bulk(*pPdu, *pTarget, non_reps, max_reps);
		if (status != SNMP_CLASS_SUCCESS)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "SNMP GetBulk(group:%s) error,err:%s,tagname:%s,tagaddr:%s",
				pTagGroup->szAutoGroupName, pSnmp->error_msg(status), pTag->szName, pTag->szAddress);
			UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_BAD_UNKNOWN_REASON, "unknown reason");
			break;
		}
	}
	string strJsonValue = jsonVal.toStyledString();
	pkUpdate(g_hPKData, pTag->szName, (char *)strJsonValue.c_str(), 0);
	Drv_LogMessage(PK_LOGLEVEL_INFO, "获取到bulk一批值,tagname:%s,addr:%s,值的个数:%d", pTag->szName, pTag->szAddress,jsonVal.size());
	return 0;
}

// 将tag点的值变成字符串
int TagVal2String(PKTAG *pTag, char *szBuff, int nDataLen, char *szOutString, int nOutLen)
{
	if(pTag->nDataType == TAG_DT_BOOL)
		sprintf(szOutString, "%d", (*((char *)&szBuff[0]))==0?0:1);
	else if(pTag->nDataType == TAG_DT_CHAR)
		sprintf(szOutString, "%c", *((char *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_UCHAR)
		sprintf(szOutString, "%c", *((unsigned char *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_INT16)
		sprintf(szOutString, "%d", *((short *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_UINT16)
		sprintf(szOutString, "%d", *((unsigned short *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_INT32)
		sprintf(szOutString, "%d", *((int *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_UINT32)
		sprintf(szOutString, "%u", *((unsigned int *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_INT64)
		sprintf(szOutString, "%l", *((int64_t *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_UINT64)
		sprintf(szOutString, "%l", *((uint64_t *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_FLOAT)
		sprintf(szOutString, "%f", *((float *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_DOUBLE)
		sprintf(szOutString, "%f", *((double *)&szBuff[0]));
	else if(pTag->nDataType == TAG_DT_TEXT)
		sprintf(szOutString, "%s", szBuff);
	else
		return -1;
	
	return strlen(szOutString);
}

// determine the smi type and get a value from the user
int TagData2SnmpValue(SmiUINT32 snmpDataType, PKTAG *pTag, char *szBuff, int nDataLen, Vb &vb)
{

	if (snmpDataType == sNMP_SYNTAX_NOSUCHINSTANCE)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,snmp Instance does not exists, snmp valid.", pTag->szName, pTag->szAddress);
		return -1;
	}

	char szOutStrVal[256] = {0};
	if(TagVal2String(pTag, szBuff, nDataLen, szOutStrVal, sizeof(szOutStrVal)) < 0){
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,datatype:%d, invalid.", pTag->szName, pTag->szAddress, pTag->nDataType);
		return -1;
	}

	switch (snmpDataType) {
		// octet string
	case sNMP_SYNTAX_OCTETS:
		{
			OctetStr octetstr(szOutStrVal);
			if (octetstr.valid()) {
				vb.set_value( octetstr);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s,Invalid Octet value.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
		// IP Address
	case sNMP_SYNTAX_IPADDR:
		{
			IpAddress ipaddress(szOutStrVal);
			if (ipaddress.valid()) {
				vb.set_value( ipaddress);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s,Invalid IP Address.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
		// Oid
	case sNMP_SYNTAX_OID:
		{
			Oid oid(szOutStrVal);
			if (oid.valid()) {
				vb.set_value(oid);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s,Invalid Oid.", pTag->szName, pTag->szAddress,szOutStrVal);
				return -1;
			}
		}
		break;
		// TimeTicks
	case sNMP_SYNTAX_TIMETICKS:
		{
			unsigned long i;
			i = ::atol(szOutStrVal);
			TimeTicks timeticks( i);
			if ( timeticks.valid()) {
				vb.set_value( timeticks);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, Invalid TimeTicks.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
		// Gauge32
	case sNMP_SYNTAX_GAUGE32:
		{
			unsigned long i;
			i = ::atol( szOutStrVal);
			Gauge32 gauge32(i); 
			if ( gauge32.valid()) {
				vb.set_value( gauge32);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, Invalid Gauge32.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
	case sNMP_SYNTAX_CNTR32:
		{
			unsigned long i;
			i = ::atol( szOutStrVal);
			Counter32 counter32(i);
			if (counter32.valid()) {
				vb.set_value( counter32);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, Invalid Counter32.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
	case sNMP_SYNTAX_CNTR64:
		{
			unsigned long i;
			i = ::atol( szOutStrVal);
			Counter64 counter64(i);
			if (counter64.valid()) {
				vb.set_value( counter64);
			}
			else {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, Invalid Counter64.", pTag->szName, pTag->szAddress, szOutStrVal);
				return -1;
			}
		}
		break;
	case sNMP_SYNTAX_INT:
		{
			long i = ::atol( szOutStrVal);
			Counter64 counter64(i);
			vb.set_value( counter64);
		}
		break;
	case sNMP_SYNTAX_NOSUCHOBJECT:
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, Invalid Counter64.", pTag->szName, pTag->szAddress, szOutStrVal);
			return -1;
		}
	default:
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, unknown datatype:%d.", pTag->szName, pTag->szAddress, snmpDataType);
		return -1;
	}

	Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s,value:%s, ready to control.", pTag->szName, pTag->szAddress, szOutStrVal);
	return 0;
}

/**
 *  当有控制命令时该函数被调用.
 *  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
 *
 *  @version     12/11/2012    Initial Version.
 */
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	char szBinValue[PK_TAGDATA_MAXLEN] = {0};
	int nBinValueLen = 0;
	Drv_TagValStr2Bin(pTag, szStrValue, szBinValue, sizeof(szBinValue), &nBinValueLen);

	Pdu pdu;                               // construct a Pdu object
	Vb vb;                                 // construct a Vb object
	vb.set_oid(pTag->szAddress);  // set the Oid portion of the Vb
	pdu += vb;   
	snmp_version version = (snmp_version)pDevice->nUserData[DEV_N_USERDATA_VERSION];
	if (version == version3) {
		int securityLevel = pDevice->nUserData[DEV_N_USERDATA_SECURITYLEVEL];
		OctetStr contextName(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTNAME]);
		OctetStr contextEngineID;
		contextEngineID.from_hex_string(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTENGINEID]);
		pdu.set_security_level( securityLevel);
		pdu.set_context_name (contextName);
		pdu.set_context_engine_id(contextEngineID);
	}

	SnmpTarget *pTarget = (SnmpTarget *)pDevice->pUserData[DEV_N_USERDATA_TARGET];
	Snmp *pSnmp = (Snmp *)pDevice->pUserData[DEV_P_USERDATA_SNMP];
	int non_reps = pDevice->nUserData[DEV_N_USERDATA_NONREPS];
	int max_reps = pDevice->nUserData[DEV_N_USERDATA_MAX_REPS];

	// first get the variabel to determine its type
	int status = pSnmp->get(pdu,*pTarget);
	if(status != SNMP_CLASS_SUCCESS) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s, cannt determine snmp datatype.", pTag->szName, pTag->szAddress);
		return -1;
	}
	pdu.get_vb(vb,0); // 获取到这个oid的值及类型
	
	vb.set_oid(pTag->szAddress);           // use the same oid as the inquire
	int nRet = TagData2SnmpValue(vb.get_syntax(), pTag, szBinValue, nBinValueLen, vb);
	if(nRet != 0)
		return nRet;

	Pdu setpdu;
	if (version == version3) {
		int securityLevel = pDevice->nUserData[DEV_N_USERDATA_SECURITYLEVEL];
		OctetStr contextName(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTNAME]);
		OctetStr contextEngineID;
		contextEngineID.from_hex_string(pDevice->szUserData[DEV_SZ_USERDATA_CONTEXTENGINEID]);
		setpdu.set_security_level( securityLevel);
		setpdu.set_context_name (contextName);
		setpdu.set_context_engine_id(contextEngineID);
	}
	vb.set_oid(pTag->szAddress);           // use the same oid as the inquir
	setpdu += vb; 
	status = pSnmp->set(setpdu, *pTarget);
	if(status == 0){
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OnControl, tag:%s,address:%s, value:%s,write success.", pTag->szName, pTag->szAddress, vb.get_printable_value());
	}
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, tag:%s,address:%s, value:%s,write failed.error:%s", pTag->szName, pTag->szAddress, vb.get_printable_value(), pSnmp->error_msg( status));

	return nRet;
}
	