#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <stdlib.h>
#include <stdio.h>
#include "ace/ACE.h"
#include "ace/Time_Value.h"
#include "avglobal.h"
#include "dhnetsdk.h"
#include "dhconfigsdk.h"

//#include "video/VideoPlugin.h"

#define DVR_CHANNELNUM                  16
#define DVR_TIME_OUT					3000
#define DVR_CONNECT					"dvr:state"
#define DISKTOTAL					"disk:total"
#define DISKFREE					"disk:free"
#define DISKFREEPERCENT					"disk:freepercent"
#define DEV_PARAM_CALLFAINUM            2   // fail num of call
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
//void CALLBACK AutoConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

int GetUserInfo(PKDEVICE *pDevice, string& strIP, WORD&  port, string& username, string& password);
void QueryDiskInfo(PKDEVICE *pDevice, long nLoginID, vector<PKTAG *> &vecTag);
int QueryCamerafault(PKDEVICE *pDevice, long nLoginID, vector<PKTAG *> &vecTag);

int CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
    Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Driver:(%s),Device:(%s),vecTags.size:(%d)", pDevice->pDriver->szName, pDevice->szName, pDevice->nTagNum);
    PKTIMER timer;
    timer.nPeriodMS = 10000;
    Drv_CreateTimer(pDevice, &timer);
    bool ret = CLIENT_Init(DisConnectFunc, (long)pDevice);
    if (!ret)
    {
        return -1;
    }

    return 0;
}

/*

*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
    CLIENT_StopListen(pDevice->nUserData[0]);
    CLIENT_Logout(pDevice->nUserData[0]);
    CLIENT_Cleanup();
    return 0;
}

int UpdateAllTagsBad(PKDEVICE *pDevice)
{
    for (int i = 0; i < pDevice->nTagNum; i++)
    {
        PKTAG *pTag = pDevice->ppTags[i];
        if (strcmp(pTag->szAddress ,DVR_CONNECT) == 0 )
        {
            char szValue[256] = {0};
            int iValue = 1;
            sprintf( szValue, "%d",iValue);
            Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);
        }
        else
        {
			Drv_SetTagData_Text(pTag, "*", 0, 0, -100);
        }
    }
    int nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
	return nRet;
}

// re login
int LoginToDVR(PKDEVICE *pDevice)
{
    NET_DEVICEINFO DeviceInfo;
    int errNum = 0;

    CLIENT_SetDVRMessCallBack(MessCallBack, (long)pDevice);
    CLIENT_SetConnectTime(DVR_TIME_OUT, NULL);
    string strIP,username,password;
    WORD Port;
    GetUserInfo(pDevice, strIP, Port, username, password);
    if(strIP.empty() || Port == 0)
    {
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_Login(%s,%d), ip or port is null, Driver = %s,Device =%s",strIP.c_str(), Port,pDevice->pDriver->szName, pDevice->szName);
        pDevice->nUserData[0] = 0;
        return 0;
    }

    pDevice->nUserData[DEV_PARAM_CALLFAINUM] = 0;
    long nLoginID = CLIENT_Login(strIP.c_str(), Port, username.c_str(), password.c_str(), &DeviceInfo, &errNum);
    if (nLoginID ==0)
    {
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_Login(%s,%d,%s,%s), return LoginID=%d<=0, Driver = %s,Device =%s",strIP.c_str(), Port, username.c_str(), password.c_str(), nLoginID,pDevice->pDriver->szName, pDevice->szName);
        UpdateAllTagsBad(pDevice);
        pDevice->nUserData[0] = 0;
        return 0;
    }

    CLIENT_StartListenEx(nLoginID);
    pDevice->nUserData[0] = nLoginID;
    return nLoginID;
}

PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
   if (pDevice->nUserData[0] == 0)
    {
        int nLoginRet = LoginToDVR(pDevice);
        if(nLoginRet == 0)
        {
           return -2;
        }
    }

    long nLoginID = pDevice->nUserData[0];
    if(nLoginID == 0)
        return -1;


    vector<PKTAG *> vecTags;
    QueryDiskInfo(pDevice, nLoginID , vecTags);

    QueryCamerafault(pDevice, nLoginID, vecTags);

    Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());

    return 0;
}


void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(dwUser == 0)
    {
        return;
    }
    //string strTemp;
    //strTemp = IDS_STRING101;
    //Drv_LogMessage(PK_LOGLEVEL_INFO,strTemp.c_str(),pchDVRIP,CLIENT_GetLastError());

}
/*void CALLBACK AutoConnectFunc(LONG lLoginID,char *pchDVRIP,LONG nDVRPort,DWORD dwUser)
{
    cout<<"Reconnect success."<<endl;
    return;
}*/

void CheckReconnect(PKDEVICE *pDevice)
{
    if(pDevice->nUserData[DEV_PARAM_CALLFAINUM] > 10)
    {
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "%s call fail num:%d>10, try to relogin...", pDevice->szName, pDevice->nUserData[DEV_PARAM_CALLFAINUM]);
        LoginToDVR(pDevice);
    }
}

//-------------------------check integrity
int GetUserInfo(PKDEVICE *pDevice, string& strIP, WORD&  port, string& username, string& password)
{
    string connParms = pDevice->szConnParam;
    int nIndexStx = connParms.find("ip=");
    if(nIndexStx < 0)
    {
        Drv_LogMessage(PK_LOGLEVEL_CRITICAL, "connection param must have ip=,assume all is IP,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
        return -1;
    }
    int nIndexEnd = connParms.find_first_of(";");
    if(nIndexEnd < 0)
    {
        Drv_LogMessage(PK_LOGLEVEL_CRITICAL, "connection param must have ';',assume all is IP,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
        return -1;
    }

    strIP = connParms.substr(nIndexStx+3, nIndexEnd - nIndexStx - 3);
    string strPortString = connParms.substr(nIndexEnd+1, -1);
    nIndexStx = strPortString.find("port=");
    if(nIndexStx < 0)
    {
        Drv_LogMessage(PK_LOGLEVEL_CRITICAL, "connection param must have port=,assume all is port,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
        return -1;
    }

    string strport = strPortString.substr(nIndexStx + 5);
    port = atoi(strport.c_str());

    username = pDevice->szParam1;
    if(username.empty())
        username = "admin";

    password  = pDevice->szParam2;;
    if(password.empty())
        password = "admin";

	return 0;
}

void QueryDiskInfo(PKDEVICE *pDevice, long nLoginID, vector<PKTAG *> &vecTags)
{
    int iAllTotalSpace =0;
    int iAllFreeSpace  = 0;
    char  buffer[2048] = {0};
    LPDH_HARDDISK_STATE HddInfo;

    int HDDsize = 0;
    bool bDevDisk = CLIENT_QueryDevState(nLoginID, DH_DEVSTATE_DISK, buffer, 2048, &HDDsize, 3000);
    HddInfo = (LPDH_HARDDISK_STATE)buffer;
    int nDiskNum = HddInfo->dwDiskNum;
    if(bDevDisk)
    {
        pDevice->nUserData[DEV_PARAM_CALLFAINUM] = 0;

        if(nDiskNum > 0)
        {
            for (int i=0;i<nDiskNum;i++)
            {
                iAllTotalSpace +=HddInfo->stDisks[i].dwVolume;
                iAllFreeSpace  +=HddInfo->stDisks[i].dwFreeSpace;
            }
        }
        else
        {
            iAllTotalSpace = -1024;
            iAllFreeSpace = -1024;
            Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_QueryDevState success,but DiskNum<=0 ,iAllTotalSpace = %d,iAllFreeSpace =%d,Driver = %s,Device =%s",iAllTotalSpace, iAllFreeSpace,pDevice->pDriver->szName, pDevice->szName);

        }
    }
    else
    {
        pDevice->nUserData[DEV_PARAM_CALLFAINUM] ++;
        CheckReconnect(pDevice);

        iAllTotalSpace = -2048;
        iAllFreeSpace = -2048;
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_QueryDisk fail,iAllTotalSpace = %d,iAllFreeSpace =%d,Driver = %s,Device =%s",iAllTotalSpace, iAllFreeSpace,pDevice->pDriver->szName, pDevice->szName);

    }

    iAllTotalSpace = iAllTotalSpace/1024;
    iAllFreeSpace  = iAllFreeSpace/1024;

    for (int i = 0; i < pDevice->nTagNum; i++)
    {
        PKTAG *pTag = pDevice->ppTags[i];
        char szValue[256] = {0};
        if (strcmp(pTag->szAddress ,DISKTOTAL) == 0 )
        {
            sprintf( szValue, "%d",iAllTotalSpace);
        }
        else if (strcmp(pTag->szAddress ,DISKFREE) == 0 )
        {
            sprintf(szValue, "%d",iAllFreeSpace);
        }
        else if (strcmp(pTag->szAddress ,DISKFREEPERCENT) == 0)
        {
            if ((iAllTotalSpace == 0) || (iAllFreeSpace == 0)){
                char pbuf[10] = "0%";
                sprintf(szValue, "%s",pbuf);
            }
            else if ((iAllTotalSpace != 0) || (iAllFreeSpace != 0))
            {
                float di = ((float)iAllFreeSpace/(float)iAllTotalSpace)*100;
                char pbuf[20] = {0};
                sprintf(pbuf, "%.2f",di);
                string buffer = pbuf ;
                buffer = buffer+"%";
                sprintf(szValue, "%s",(char*)buffer.c_str());
            }
        }
        if(!bDevDisk)
        {
			Drv_SetTagData_Text(pTag, "*", 0, 0, -1);
            Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_QueryDisk fail * ,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);

        }
        else
			Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);

        vecTags.push_back(pTag);
    }
}

//
int QueryCamerafault(PKDEVICE *pDevice, long nLoginID, vector<PKTAG *> &vecTag)
{
    int nMaxChannelNum = DVR_CHANNELNUM;
    NET_IN_GET_CAMERA_STATEINFO InputCamBuf ;
    InputCamBuf.dwSize = sizeof(NET_IN_GET_CAMERA_STATEINFO);
    InputCamBuf.bGetAllFlag = TRUE;

    NET_OUT_GET_CAMERA_STATEINFO OutCamBuf ;
    OutCamBuf.dwSize =  sizeof(NET_OUT_GET_CAMERA_STATEINFO);
    OutCamBuf.nMaxNum = nMaxChannelNum;
    OutCamBuf.pCameraStateInfo = (NET_CAMERA_STATE_INFO*)malloc(nMaxChannelNum*sizeof(NET_CAMERA_STATE_INFO)) ;
    bool bQuerySuccess = false;
    if(OutCamBuf.pCameraStateInfo != NULL)
    {
        bQuerySuccess = CLIENT_QueryDevInfo(nLoginID, NET_QUERY_GET_CAMERA_STATE, &InputCamBuf, &OutCamBuf, NULL, 3000);
        if(bQuerySuccess)
        {
            pDevice->nUserData[DEV_PARAM_CALLFAINUM] = 0;
            NET_CAMERA_STATE_INFO *info = OutCamBuf.pCameraStateInfo;
            bool bFlag = false;
            for( int i= 0; i< nMaxChannelNum; i++ ,info++)
            {
                char szAddress[32] = {0};
                sprintf(szAddress, "channel:%d:state", i+1);
                PKTAG * vecTagChannel[100];
                int nTagChannelNum = Drv_GetTagsByAddr(pDevice, szAddress, vecTagChannel, 100);
                int nStatus = 0;
                if(info->emConnectionState == EM_CAMERA_STATE_TYPE_UNCONNECT ){
                    nStatus = 0;
                    bFlag = true;
                }else if(info->emConnectionState == EM_CAMERA_STATE_TYPE_CONNECTING ){
                    nStatus = 1;
                  //  }else if(info->emConnectionState == EM_CAMERA_STATE_TYPE_CONNECTED ){
                  //      status[i] = 2;
                }else
                    nStatus = 2;

                char szValue[32] = {0};
                sprintf(szValue, "%d", nStatus);
				for (int j = 0; j < nTagChannelNum; j++)
                {
                    PKTAG *pTag = vecTagChannel[j];
                    if(bQuerySuccess)
						Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);
                    else
                    {
						Drv_SetTagData_Text(pTag, "-1", 0, 0, -1);
                        Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_Query Camera fail ,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
                    }
                    vecTag.push_back(pTag);
                }
            }
            if(bFlag)
            {
                PKTAG *vecTagCam[100];
                char szCamAddress[32] = {0};
                strcpy(szCamAddress, "camera:alarm");

                int nTagCamNum = Drv_GetTagsByAddr(pDevice, szCamAddress, vecTagCam, 100);
                char nCamAlarmValue = '1';
                char szCamValue[32] = {0};
                sprintf(szCamValue, "%d", nCamAlarmValue);
				for (int j = 0; j < nTagCamNum; j++)
                {
                    PKTAG *pTag = vecTagCam[j];
                    if(bQuerySuccess)
						Drv_SetTagData_Text(pTag, szCamValue, 0, 0, 0);
                    else
                    {
						Drv_SetTagData_Text(pTag, "-1", 0, 0, -1);
                        Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_Query Camera fail ,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
                    }
                    vecTag.push_back(pTag);
                }
            }
        }
        else
        {
            Drv_LogMessage(PK_LOGLEVEL_ERROR, "CLIENT_Query Camera fail,Driver = %s,Device =%s",pDevice->pDriver->szName, pDevice->szName);
            pDevice->nUserData[DEV_PARAM_CALLFAINUM] ++;
            CheckReconnect(pDevice);
        }
        free(OutCamBuf.pCameraStateInfo);
    }

    return 0;
}

int CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    PKDEVICE *pDevice = (PKDEVICE*)dwUser;
    vector<PKTAG *> vecTags;

    switch(lCommand)
    {
   // case DH_MOTION_ALARM_EX:
    case DH_VIDEOLOST_ALARM_EX:
   // case DH_SHELTER_ALARM_EX:
    {
        char szAlmType[32] = {0};
    //    if(lCommand == DH_MOTION_ALARM_EX)
       //     strcpy(szAlmType, "dynamic");
        if(lCommand == DH_VIDEOLOST_ALARM_EX)
            strcpy(szAlmType, "lose");
      //  else if(lCommand == DH_SHELTER_ALARM_EX)
       //     strcpy(szAlmType, "shelter");

        int nMaxChannel = dwBufLen;
        for( int i= 0; i< nMaxChannel; i++)
        {
            char szAddress[32] = {0};
            sprintf(szAddress, "channel:%d:%s", i+1, szAlmType);
            PKTAG * vecTagChannel[100];
            int nTagChannelNum = Drv_GetTagsByAddr(pDevice, szAddress, vecTagChannel, 100);

            char nAlarmValue = *(pBuf + i);
            char szValue[32] = {0};
            sprintf(szValue, "%d", nAlarmValue);
			for (int j = 0; j < nTagChannelNum; j++)
            {
                PKTAG *pTag = vecTagChannel[j];
                Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);
                vecTags.push_back(pTag);
            }
        }

        PKTAG * vecTagCam[100];
        char szCamAddress[32] = {0};
        strcpy(szCamAddress, "camera:alarm");
        int nTagCamNum = Drv_GetTagsByAddr(pDevice, szCamAddress, vecTagCam, 100);
        char nCamAlarmValue = '1';
        char szCamValue[32] = {0};
        sprintf(szCamValue, "%d", nCamAlarmValue);
		for (int j = 0; j < nTagCamNum; j++)
        {
            PKTAG *pTag = vecTagCam[j];
            Drv_SetTagData_Text(pTag, szCamValue, 0, 0, 0);
            vecTags.push_back(pTag);
        }
    }
    break;
    case DH_DISKFULL_ALARM_EX:
    case DH_DISKERROR_ALARM_EX:
    {
        char szAlmType[32] = {0};
        if(lCommand == DH_DISKFULL_ALARM_EX)
            strcpy(szAlmType, "diskfull");
        else if(lCommand == DH_DISKERROR_ALARM_EX)
            strcpy(szAlmType, "diskerror");

        char szAddress[32] = {0};
        sprintf(szAddress, "alarm:%s", szAlmType);
        PKTAG * vecTagChannel[100];
        int nTagChannelNum = Drv_GetTagsByAddr(pDevice, szAddress, vecTagChannel, 100);

        char nAlarmValue = *pBuf;
        char szValue[32] = {0};
        sprintf(szValue, "%d", nAlarmValue);
		for (int j = 0; j < nTagChannelNum; j++)
        {
            PKTAG *pTag = vecTagChannel[j];
            Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);
            vecTags.push_back(pTag);
        }

        memset(szAddress,0,32);
        sprintf(szAddress, "%s", "disk:alarm");
        int nTagCount2 = Drv_GetTagsByAddr(pDevice, szAddress, vecTagChannel, 100);
        nAlarmValue = '1';
        memset(szValue,0,32);
        sprintf(szValue, "%d", nAlarmValue);
		for (int j = 0; j < nTagCount2; j++)
        {
              PKTAG *pTag = vecTagChannel[j];
              Drv_SetTagData_Text(pTag, szValue, 0, 0, 0);
              vecTags.push_back(pTag);
        }

    }
    break;

    default:
        break;
    }

    if(vecTags.size() > 0)
        Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());
    return TRUE;

}
