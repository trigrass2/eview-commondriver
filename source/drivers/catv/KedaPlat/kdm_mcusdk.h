#ifndef _KDM_MCUSDK_INCLUDE_
#define _KDM_MCUSDK_INCLUDE_

#include "McuSdkType.h"

#ifdef KDM_MCUSDK_EXPORTS
#define KDM_MCUSDK_API __declspec(dllexport)
#else
#define KDM_MCUSDK_API __declspec(dllimport)
#endif

#if defined (_MAC_IOS_) ||defined (_KDM_LINUX_)
#define STDCALL 
#define KDM_MCUSDK_API 
#else
#define STDCALL __stdcall
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	
	// create sdk,destroy sdk.
    KDM_MCUSDK_API void* STDCALL	Kdm_CreateMcuSdk();
	KDM_MCUSDK_API void	STDCALL		Kdm_DestroyMcuSdk(void *pMcuHandle);
	
	// init Modual.
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_Init(void *pMcuHandle);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_Cleanup(void *pMcuHandle);

	// modual select(operation).
	KDM_MCUSDK_API	EBussinessMod STDCALL 	Kdm_PlatTypeDetect(void *pMcuHandle,const char *strnsAddr,u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_ModualSelect(void *pMcuHandle, EBussinessMod emBuss, EStreamMod emStream, EDecoderMod emDec);
    
    // get version.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetSdkVersion(void *pMcuHandle, char* pchVersion, char *pchUpdateurl);
    
    // set event callback function.
    KDM_MCUSDK_API u32 STDCALL		Kdm_SetSDKEventCallback(void *pMcuHandle, McuSdkEvent_Callback EventCBFunc, u32 dwUserData);
    
	// login/logout plat(cmu).
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_Login(void *pMcuHandle, const char *strUser, const char *strPassword, const char *strnsAddr, const char *strClientType, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_Logout(void *pMcuHandle, u32 *pErrorCode);
    
    // get group.
    KDM_MCUSDK_API u32 STDCALL 		Kdm_GetGroupByGroup(void *pMcuHandle, GroupID gourpid, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetGroupNext(void *pMcuHandle, u32 dwTaskID, GROUPINFO* groupInfo, u32 *pErrorCode);
    
    // get device.
    KDM_MCUSDK_API u32 STDCALL 		Kdm_GetDeviceByGroup(void *pMcuHandle, GroupID gourpid, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetDeviceNext(void *pMcuHandle, u32 dwTaskID, DEVICEINFO* deviceInfo, u32 *pErrorCode);
    
    // device Status.
	// first,set device status callback function. second,subscript device. device status will notify by the call function.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SetDevStatusCallback(void *pMcuHandle, DevStatus_Callback cbFunc, u32 deUserData);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SubscriptDeviceStatus(void *pMcuHandle, TSUBSDEVS vctDeviceID, ESubscriptInfo emSbs, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_UnSubscriptDeviceStatus(void *pMcuHandle, TSUBSDEVS vctDeviceID, ESubscriptInfo emSbs, u32 *pErrorCode);
    
	// get device GBID.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetDeviceGBID(void *pMcuHandle, const DEVCHN tDevChn, DEVCHN &tDevGbID,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetDevicekdmNO(void *pMcuHandle, const DEVCHN tDevChn,DeviceKDMNO& tDevkdmNo,u32 *pErrorCode);

    // ptz.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SendPtzControl(void *pMcuHandle, DEVCHN tDevChn, PTZCMD tPtzCmd, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SendPointMoveControl(void *pMcuHandle, DEVCHN tDevChn, POINTMOVE tPointCmd, u32 *pErrorCode );
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SendPtzPreSet(void *pMcuHandle, DEVCHN tDevChn,int tPreSetPosition,TPTZAUTHORITY tPtzAuthority,u32 *pErrCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TaskWatcher(void *pMcuHandle,DEVCHN tDevChn,TWatchTask tWatchTask,u32 *pErrCode);
	KDM_MCUSDK_API BOOL32 STDCALL   Kdm_PrePosWatcher(void *pMcuHandle,DEVCHN tDevChn,TWatchPrePos tWatchPrePos,u32 *pErrCode);
	// set stream quality.
	// the setting will effective in next real stream.
	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetHighDefinitionValue(void *pMcuHandle);
    KDM_MCUSDK_API void   STDCALL 	Kdm_SetHighDefinitionValue(void *pMcuHandle, BOOL32 temHighDefinition);

    // set feature code.
	// the feature code is used in MSS to distribute server.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SetFeatureCode(void *pMcuHandle, const char* pchFeatureCode);

    // browse real stream.
	// return value is PlayID,the PlayID is used in stop/local record/snapshot...
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SetStreamPattern(void *pMcuHandle,EStreamFlowPattern eStreamPattern,u32 *pErrorCode);
    KDM_MCUSDK_API u32 STDCALL 		Kdm_StartRealPlay(void *pMcuHandle, DEVCHN tDevChn, SPARAM tStreamParam, CB_SDKMEDIA tCbSdkMedia, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_StopRealPlay(void *pMcuHandle, u32 dwPlayID, u32 *pErrorCode);	
	KDM_MCUSDK_API u32 STDCALL	    Kdm_SetUpStreamTrans(void *pMcuHandle,DEVCHN tDevChn,TStreamTransLocal tStrTransLoc,TStreamTransRemote& tStrTransRemote,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StartStreamTrans(void *pMcuHandle,u32 StrTransId,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StopStreamTrans(void *pMcuHandle,u32 StrTransId,u32 *pErrorCode);

	// PreLoadRecordInfo.
    KDM_MCUSDK_API u32 STDCALL    	Kdm_CreateRecordTask(void *pMcuHandle, DEVCHN tDevChn, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_PreLoadRecord(void *pMcuHandle, u32 dwTaskID, TPeriod tPreLoadTime, eRecordType ERecType,u32 *pErrorCode);   

	//get record info
	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetRecordNext(void *pMcuHandle, u32 dwTaskID, u32 dwSeekTime, TRecordInfo *pRecordInfo, u32 *pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_DestroyRecordTask(void *pMcuHandle, u32 dwTaskID);

    // record play.
    KDM_MCUSDK_API u32 STDCALL 	  	Kdm_StartRecordPlay(void *pMcuHandle, u32 dwTaskID, u32 dwPlayTime, SPARAM stSparam,CB_SDKMEDIA tCbSdkMedia,u32 *pErrorCode);  
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_StopRecordPlay(void *pMcuHandle, u32 dwPlayID, u32 *pErrorCode); 
	KDM_MCUSDK_API u32 STDCALL	    Kdm_RecordPlayBySETime(void *pMcuHandle, u32 dwTaskID, TRecordPlayInfo tRecordPlayInfo, CB_SDKMEDIA tCbSdkMedia,u32 *pErrorCode);
	//vcr
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_RecordPlayCtrl(void *pMcuHandle,TREPCTLINFO stRecPlyInfo,u32 *pErrorCode);
	
	//record download
	KDM_MCUSDK_API u32  STDCALL 	Kdm_StartRecordDownLoad(void *pMcuHandle,u32 dwTaskID, TRecordDownloadInfo tDownloadInfo, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32  STDCALL  Kdm_StopRecordDownLoad(void *pMcuHandle,u32 dwPlayID, u32 *pErrorCode);
	

	// local Record.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_StartLocalRecord(void *pMcuHandle, u32 dwPlayID, const char *strFileName, ELocalRecType emFileType);
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_StopLocalRecord(void *pMcuHandle, u32 dwPlayID);
    
    // snapshot.
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_SaveSnapshot(void *pMcuHandle, u32 dwPlayID, const char *strPicName, EPictureType emPicType);
    
    // refresh play window (used in ios).
    KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_RefreshPlayWnd(void *pMcuHandle, u32 dwPlayID);

	// set Audio.
	KDM_MCUSDK_API	BOOL32 STDCALL	Kdm_SetAudioEnable(void *pMcuHandle, u32 dwPlayID);
	KDM_MCUSDK_API	BOOL32 STDCALL	Kdm_SetAudioDisable(void *pMcuHandle, u32 dwPlayID);

	// set video Osd
	KDM_MCUSDK_API BOOL32 STDCALL   Kdm_SetVideoOSD(void *pMcuHandle, DEVCHN tDevChn, TVidOSD tInfo, u32* pErrorCode);
	
	// plat rec.
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StartPlatRec(void *pMcuHandle, DEVCHN tDevChn, u32* pErrorCode);
	KDM_MCUSDK_API	BOOL32 STDCALL	Kdm_StopPlatRec(void *pMcuHandle, DEVCHN tDevChn, u32* pErrorCode);
	
	// pu rec.
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StartPuRec(void *pMcuHandle, DEVCHN tDevChn, u32* pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StopPuRec(void *pMcuHandle, DEVCHN tDevChn, u32* pErrorCode);
	
	// Search Device.
	// first,post Search Request. Second,after receive search finished event,Get Device/Group. 
	KDM_MCUSDK_API	BOOL32	STDCALL	Kdm_SearchDvcReq(void *pMcuHandle, const char* SearchDvcName,int SearchResLimit,u32* pErrorCode);
	KDM_MCUSDK_API	BOOL32  STDCALL	Kdm_GetSearchDvcInfo(void *pMcuHandle, StSearchDvcRspInfo &temSearchDvcInfo,u32* pErrorCode);
	KDM_MCUSDK_API	BOOL32  STDCALL Kdm_GetGroupInfo(void *pMcuHandle,GroupID tGroupID, GROUPINFO &tGroupInfo,u32* pErrorCode);
	
	//add for LogFile.
	KDM_MCUSDK_API BOOL32  STDCALL 	Kdm_SetSaveLogFile(void *pMcuHandle,u32 dwLogFlag,const char* SaveLogFileDirectName );
	KDM_MCUSDK_API BOOL32  STDCALL  Kdm_SetScreenShowLog(void *pMcuHandle,u32 dwShowLogLev);

	// get device video source status.
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetDevSrcStatus(void *pMcuHandle, DEVCHN tDevSrc, DEVSRC_ST* srcStatus, u32 *pErrorCode);
    
	// send keyframe request.
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SendKeyFrame(void *pMcuHandle, DEVCHN tDevChn, u32 *pErrorCode);

	// send transdata request.
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SendTransData(void *pMcuHandle, DEVCHN tDevChn, TTransChnData *pTransData, u32 *pErrorCode);

	// get/set pu encoder parameter.
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetPuEncoderParam(void *pMcuHandle, DEVCHN tDevChn, TEncoderParamTotal& pEncoderParamTotal,u32* pErrorCode);
    KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SetPuEncoderParam(void *pMcuHandle, DEVCHN tDevChn, TEncoderParam *pEncoderParam,u32* pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL   Kdm_GetPuAudioParams(void *pMcuHandle, DEVCHN tDevChn, TAudioParams& tAudParam, u32* pErrorCode);

	//get dev gps info
	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_GetDevGpsInfo(void *pMcuHandle, DEVCHN tDevChn, DeviceGPSInfo& tDevGpsInfo,u32* pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetDevLatiLongTude(void *pMcuHandle,DEVCHN tDevChn,TDevLatiLongTude& tDevLatiLongtude,u32* pErrorCode);

	//get PLaying Chn audio and video info
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetPlayingChnInfo(void *pMcuHandle,u32 dwPlayID,TTPlyingChnAVInfo& tPlayingChnInfo,u32* pErrorCode);
	//start audio call
	KDM_MCUSDK_API u32 	  STDCALL 	Kdm_StartVoiceCall(void *pMcuHandle,DEVCHN temDevchn,EAudioEncType temAudioEncType,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_StopVoiceCall(void *pMcuHandle,u32 VoiceCallId,u32 *pErrorCode);

	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetTVWallTotal(void *pMcuHandle,int& tvWallTotal,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetTVWall(void *pMcuHandle,u32 tvWallIdx,TTvWallInfoData& tTvWallInfo,u32 *pErrorCode);
	KDM_MCUSDK_API char* STDCALL	Kdm_CreateTVWall(void *pMcuHandle,TTvWallInfoData CreatTvWallReq,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_ModifyTVWall(void *pMcuHandle,TTvWallInfoData tTvWallInfo,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_DelTVWall(void *pMcuHandle,char* tvWallId,char* tvWallName,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetTVWallById(void *pMcuHandle,char* tvWallId,TTvWallInfoData& tTvWallInfo,u32 *pErrorCode);
	KDM_MCUSDK_API u32 STDCALL 		Kdm_GetTVDivNum(void *pMcuHandle,char* tvWallId,int tvId, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SetTVDivNum(void *pMcuHandle,TTvDivNumData tvSetDivNumData,u32 *pErrorCode);

	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallStartPlay(void *pMcuHandle,TTvWallPlayData tvWallStartPlyData,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallStopPlay(void *pMcuHandle,TTvWallPlayData tvWallStopPlyData,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_TVWallStartPlayRecord(void *pMcuHandle,u32 dwPlayID,TTvWallCommonData tvWallComData,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallStopPlayRecord(void *pMcuHandle,u32 dwPlayID,TTvWallCommonData tvWallComData,u32 *pErrorCode);

	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_TVWallQuerySchemeTotal(void *pMcuHandle,char* tvWallId,int& tTvWallSchemeTotal, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL 	Kdm_TVWallQueryScheme(void *pMcuHandle,int SchIndx,TTvWallScheme& tvWallScheme, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL   Kdm_TVWallCreatScheme(void *pMcuHandle,TTvWallScheme tvWallScheme,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallModifyScheme(void *pMcuHandle,TTvWallScheme tvWallScheme, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallDelScheme(void *pMcuHandle,char* tvWallId,char* SchemeName, u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_TVWallLoadScheme(void *pMcuHandle,char* tvWallId,char* SchemeName,BOOL32 bLoadWithSave, u32 *pErrorCode);

	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_SetRCSSysParam(void *pMcuHandle,emCoverPolicy temRCSSParam,u32 *pErrorCode);
	KDM_MCUSDK_API BOOL32 STDCALL	Kdm_GetRCSSysParam(void *pMcuHandle,emCoverPolicy& temRCSSParam,u32 *pErrorCode);
	
	KDM_MCUSDK_API BOOL32 STDCALL   Kdm_SetTimeSyncServer(void *pMcuHandle, BOOL32 bSynctimeServer, BOOL32 bSendCmd, u32* pErrorCode);
#ifdef __cplusplus
}
#endif

#endif
