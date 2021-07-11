//
//  McuSdkType.h
//  McuSdk
//
//  Created by dqw on 14-5-16.
//  Copyright (c) 2014茠脥 ___kedacom___. All rights reserved.
//

#ifndef McuSdk_McuSdkType_h
#define McuSdk_McuSdkType_h

#include "kdvtype.h"
#include <string.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_WATCH_ON_NUM 8
#define ADDR_STR_LEN 64
#define MAX_ID_LEN 128
#define MAX_NAME_LEN 256

#define MAX_VID_SRC_NUM 128
#define MAX_VID_CHN_NUM 256
#define MAX_SM_STR_LEN 256
#define MAX_URL_LEN 256
#define MAX_DESC_LEN 128
#define MAX_DEV_DIR	256

#define MAX_ADDR_LEN 128
#define MAX_LOCAL_IP_NUM 10

#define MAX_REQ_DEV_SUBS_NUM 20
#define LEN_KDM_NO         32 
#define MAXNUM_SRCPU_POLL    64  //轮询最大编码器数量

#define INVALID_TASK_ID 0
#define INVALID_STREAM_INDEX (u32)0xFFFF
#define INVALID_CODEC_INDEX (u32)0xFFFF

#define MAX_STREAM_URL_NUM 20

#define MAX_DISPLAY_CONTENT_LEN 128
    
#define MAX_SUPPORT_VIDEO_TYPE_NUM 8

#define MAX_TRANSCHAN_DATA_LEN 512

#define STREAM_URL_SELECT_INDEX_INVAILED 65535	//非法的url 序列号

enum McuErrorCode
{
    RTN_OK 														=0,		//调用SDK结构成功										

	//add by xql 20140916 for startplay error
	MCU_PLAYER_ERR_MVC_CONNECT_MVS_FAILED						=10120,//MVC连接MVS时TCP链路建链失败

	MODUAL_INVALID 												=60001, // 无效模块
    TASK_INVALID 												=60002, // 无效任务
    TASK_CREATE_ERROR 											=60003, // 创建任务失败
    INPUT_ERROR 												=60004, // 输入错误
    GET_DATA_ERROR 												=60005, // 获取数据错误
    NET_ERROR													=60006, // 网络错误
    SNAPSHOT_ERROR 												=60007, //
    DGROUP_ROOT_INFO_ERROR 										=60008,//登陆时获取根平台的信息错误
	MCU_ERRCODE_LOGIN_ERR_FIRST_DOMAIN_ERR						=60010,//登陆解析第一级域名失败
	MCU_ERRCODE_LOGIN_ERR_SECOND_DOMAIN_ERR 					=60011,//登陆解析第二级域名失败
	MCU_ERRCODE_LOGIN_ERR_PLAT_DOMAIN_ERR						=60012,//登陆解析平台的域名失败
	NTP_NETWORK_ERROR											=60013,//网络错误
	GET_DEVICES_INFO_TIME_OUT									=60014,//获取设备信息超时
	

	//TCP视频播放错误码(SDK 预留100个错误码61000 - 61099)
    MCU_PLAYER_ERR_STREAM_GET_IDLE_STREAM 						=61000,//传输模块分配空闲失败
	MCU_PLAYER_ERR_DECODER_GET_IDLE_DECODER						=61001,//解码模块分配空闲失败
	MCU_PLAYER_ERR_DECODER_CREATE								=61002,//解码器创建失败
    MCU_PLAYER_ERR_DECODER_START_PLAY_STREAM					=61003,//启动解码模块失败
    MCU_PLAYER_ERR_DECODER_START_PLAY_WND						=61004,//显示模块初始化失败
	MCU_PLAYER_ERR_CONVERT_GB_DEVICED_ID						=61005,//devicedID转国标ID失败
	MCU_PLAYER_ERR_G900_INIT_FAIL								=61006,//G900模块初始化失败
	MCU_PLAYER_ERR_FROM_G900_GET_URL							=61007,//从G900获取URL失败
	MCU_PLAYER_ERR_G900_START_REQ_FAIL							=61008,//g900模块发送浏览请求失败
	MCU_PLAYER_ERR_DEVICES_OFFLINE								=61009,//设备不在线
	MCU_PLAYER_ERR_G900_URL_NOT_SUPPORT_ALL						=61010,//g900返回的url分辨率不支持
	MUC_PLAYER_ERR_NO_KEY_FRAME_COME							=61011,//码流关键桢没有过来
	
	MCU_PLAYER_ERR_CONNECT_MVC_FAIL 							=61020,//mvc连接mvs失败
	MCU_PLAYER_ERR_CONNECT_MVC_TIMEOUT							=61021,//mvc连接mvs超时
	MCU_PLAYER_ERR_DISCONNECT_MVC_NTF							=61022,//收到mvs断链通知
	
    MCU_PLAYER_ERR_G900_ERR_FAIL 								=61091,//G900错误
    MCU_PLAYER_ERR_G900_ERR_UNINIT 								=61092,//G900未初始化
    MCU_PLAYER_ERR_G900_ERR_UNCONNECT 							=61093,//未连接G900
    MCU_PLAYER_ERR_G900_ERR_PARAM 								=61094,//G900 参数错误
    MCU_PLAYER_ERR_G900_ERR_INVALID_PLAYEID						=61095,//G900 playid无效	
    MCU_PLAYER_ERR_G900_TIMEOUT									=61096,//G900 请求超时

	//UDP视频播放错误码(SDK 预留100个错误码61100 - 61199)
	MCU_PLAYER_ERR_UDP_DVC_NOT_ONLIE							=61100, //udp请求码流方式设备不在线
	MCU_PLAYER_ERR_UDP_CHN_NOT_ONLIE							=61101,	//UDP请求码流方式时视频源对应的通道全不在线
	MCU_PLAYER_ERR_UDP_GET_CHNINFO_ERR							=61102, //udp请求码流方式从sdk内获取通道信息错误
	MCU_PLAYER_ERR_UDP_SETUP_STREAM_ERR_CONNECT_PLAT			=61103,	//udp请求码流请求rtp和rtcp端口连接平台错误
	MCU_PLAYER_ERR_UDP_SETUP_STREAM_ERR_RSP_ERR					=61104,	//udp请求码流请求rtp和rtcp端口平台返回结果错误
	MCU_PLAYER_ERR_UDP_SETUP_STREAM_ERR_KEY_FRAME_CONNECT_PLAT	=61105,	//udp请求码流向平台请求关键帧连接平台错误
	MCU_PLAYER_ERR_UDP_SETUP_STREAM_ERR_KEY_FRAME_RSP_ERR		=61106,	//udp请求码流向平台请求关键帧平台返回结果错误
	MCU_PLAYER_ERR_UDP_Get_IDLE_DECODER							=61107, //UDP请求码流请求分配空闲的传输模块失败
	MCU_PLAYER_ERR_UDP_INIT_KDV_MEDIA_RCV_ERR					=61108,	//UDP请求码流初始化medianet接收模块失败
	MCU_PLAYER_ERR_UDP_START_KDV_MEDIA_RCV_ERR					=61109,	//UDP请求码流medianet开始接收码流失败
	MCU_PLAYER_ERR_UDP_START_KDV_VIDEO_PROBE_ERR				=61110,	//UDP请求码流Video穿NAT失败
	MCU_PLAYER_ERR_UDP_START_KDV_AUDIO_PROBE_ERR				=61111, //UDP请求码流Audio穿NAT失败

	//录像播放和下载相关错误码(SDK 预留100 个错误码61200-61299)
	MCU_RECORD_QUERY_RECORD_THREAD_NOT_NULL						=61200,//查询录像文件线程已存在
	MCU_ERRCODE_QUERY_RECORD_TASKID_NOT_EXITS 					=61201,//查询平台录像的taskID不存在
	MCU_ERRCODE_QUERY_RECORD_MANAGER_NULL						=61202,//查询平台录像时数据管理模块为NULL
	MCU_ERRCODE_QUERY_RECORD_MANAGER_GET_DATA_NULL 				=61203,//查询平台录像时数据管理模块获取数据为NULL
	MCU_ERRCODE_QUERY_RECORD_QUERY_REQ_FAILED 					=61204,//查询平台录像出现错误
	MCU_ERRCODE_QUERY_RECORD_QUERY_RSP_FAILED 					=61205,//查询平台录像返回结果出现错误
    MCU_ERRCODE_QUERY_RECORD_QUERY_NUM_ZERO 					=61206,//从平台20获取的录像文件个数为0
    MCU_ERRCODE_RECORD_SEEK_TIME_OUT_RANG 						=61207,//vcr操作时seektime时间跨文件
    MCU_ERRCODE_RECORD_STOP_PLAY_NTF							=61208,//收到MVS录像回放停止通知
    MCU_ERRCODE_RECORD_TYPE_WRONG								=61209,//录像类型参数错误
    MCU_ERRCODE_RECORD_GET_DEV_CHN_WRONG 						=61210,//录像回放时查询设备信息错误
    MCU_ERRCODE_RECORD_GET_TIME_RANGE 							=61211,//录像回放时获取录像开始结束时间错误
    MCU_ERRCODE_VCR_RECORD_TYPE_ERR								=61212,//录像回放vcr操作时录像类型不对
    MCU_ERRCODE_VCR_GET_PU_RECORD_MANAGER_ERR					=61213,//获取前端录像管理类实例错误
    MCU_ERRCODE_VCR_GET_CENTER_RECORD_MANAGER_ERR				=61214,//获取平台录像管理实例错误

    MCU_RECORD_DOWNLOAD_ERR_CREATE_KEDAPLAYER_ERR 				=61250,//录像下载创建kedaplayer错误
    MCU_RECORD_DOWNLOAD_ERR_PLATFORM_CONNECT_FAIL 				=61251,//录像下载连接平台出错
    MCU_RECORD_DOWNLOAD_ERR_PLATFORM_DIRCRIPTION_NULL 			=61252,//录像下载描述文件为空
	MCU_RECORD_DOWNLOAD_ERR_LOCAL_DISK_FULL						=61253,//本地磁盘空空间已满
	MCU_RECORD_DOWNLOAD_ERR_LOCAL_FULL_NAME_NULL 				=61254,//本地保存文件名为空
	MCU_RECORD_DOWNLOAD_ERR_DOWNLOAD_ERR						=61255,//录像下载网络错误
	MCU_RECORD_DOWNLOAD_ERR_LAST_RECORD_FILE_STOP_FAIL          =61256,//录像下载上一段文件停止失败
	MCU_RECORD_DOWNLOAD_ERR_NEXT_RECORD_FILE_START_FAIL         =61257,//录像下载下一段文件开始失败

	//电视墙的相关错误码(含预案和轮巡，SDK 预留100个错误码61300 - 61399)
	MCU_TVWALL_ERR_CONNECT_PLAT_ERR								=61300,//电视墙连接平台错误
	MCU_TVWALL_ERR_GET_TV_WALL_MANAGER_ERR						=61301,//电视墙和电视墙预案获取电视墙管理类实例错误
	MCU_TVWALL_ERR_GET_TV_WALL_SINGAL_ERR						=61302,//获取单个电视墙信息错误
	MCU_TVWALL_ERR_GET_TVWALL_DECODER_BIND_SIZE_ERR				=61303,//获取电视墙的解码器绑定的个数出错
	MCU_TVWALL_ERR_DECODER_SIZE_LESS_THAN_ID					=61304,//ID大于解码器的绑定个数
	MCU_TVWALL_ERR_GET_DECODER_STYLE_ERR						=61305,//获取解码器解码风格出错
	MCU_TVWALL_ERR_VIDEO_MANAGER_INSTANCE_NULL					=61306,//录像回放上墙的录像管理类的实例为空
	MCU_TVWALL_ERR_GET_PLAT_VIDEO_INFO_ERR						=61307,//获取平台录像的信息失败
	MCU_TVWALL_ERR_GET_PU_VIDEO_INFO_ERR						=61308,//获取前端录像的信息失败
	MCU_TVWALL_ERR_DECODER_BIND_NOT_FIND_ERR					=61309,//录像回放上墙解码器的绑定失败
	MCU_TVWALL_ERR_TO_TVWALL_RECORDTYPE_ERR						=61310,//录像回放录像类型错误
	MCU_TAWALL_SCHEME_ERR_GET_SINGAL_SCHEME_ERR					=61311,//电视墙预案中获取单个预案信息失败

    //搜索设备错误码(SDK预留100 个错误码 61400 - 61499)
	MCU_SERACH_DVC_THREAD_EXITS 								=61400,//查询录像文件线程已存在
	MCU_SERACH_DVC_NO_DEVICES 									=61401,//查询平台录像的taskID不存在

	//音频呼叫相关错误码(SDK 预留100  个错误码 61500 - 61599)
    MCU_CALL_PU_GET_LOCAL_IP_ERR								=61500,//音频呼叫时获取客户端本地ip错误
    MCU_CALL_PU_GET_PLAT_RSP_ERR								=61502,//音频呼叫时连接平台获取rsp错误
    MCU_CALL_PU_RECV_IP_SIZE_ZERO								=61503,//音频呼叫时平台返回的size大小出错
    MCU_CALL_PU_GET_PLAT_IP_PORT_ERR							=61504,//音频呼叫时连接平台获取端口错误


	//SOAP相关错误码(SDK预留100个错误码 64000-64099)
	
 	MCU_SDK_ERRCODE_SOAP_OK										=64000,
 	MCU_SDK_ERRCODE_SOAP_CLI_FAULT								=64001,
 	MCU_SDK_ERRCODE_SOAP_SVR_FAULT								=64002,
 	MCU_SDK_ERRCODE_SOAP_TAG_MISMATCH							=64003,
 	MCU_SDK_ERRCODE_SOAP_TYPE									=64004,
 	MCU_SDK_ERRCODE_SOAP_SYNTAX_ERROR							=64005,
 	MCU_SDK_ERRCODE_SOAP_NO_TAG									=64006,
 	MCU_SDK_ERRCODE_SOAP_IOB									=64007,
 	MCU_SDK_ERRCODE_SOAP_MUSTUNDERSTAND							=64008,
 	MCU_SDK_ERRCODE_SOAP_NAMESPACE								=64009,
 	MCU_SDK_ERRCODE_SOAP_USER_ERROR								=64010,
 	MCU_SDK_ERRCODE_SOAP_FATAL_ERROR							=64011,
 	MCU_SDK_ERRCODE_SOAP_FAULT									=64012,
 	MCU_SDK_ERRCODE_SOAP_NO_METHOD								=64013,
 	MCU_SDK_ERRCODE_SOAP_NO_DATA								=64014,
 	MCU_SDK_ERRCODE_SOAP_GET_METHOD								=64015,
 	MCU_SDK_ERRCODE_SOAP_PUT_METHOD								=64016,
 	MCU_SDK_ERRCODE_SOAP_DEL_METHOD								=64017,	/* deprecated */
 	MCU_SDK_ERRCODE_SOAP_HEAD_METHOD							=64018,	/* deprecated */
 	MCU_SDK_ERRCODE_SOAP_HTTP_METHOD							=64019,
 	MCU_SDK_ERRCODE_SOAP_EOM									=64020,
 	MCU_SDK_ERRCODE_SOAP_MOE									=64021,
 	MCU_SDK_ERRCODE_SOAP_HDR									=64022,
 	MCU_SDK_ERRCODE_SOAP_NULL									=64023,
 	MCU_SDK_ERRCODE_SOAP_DUPLICATE_ID							=64024,
 	MCU_SDK_ERRCODE_SOAP_MISSING_ID								=64025,
 	MCU_SDK_ERRCODE_SOAP_HREF									=64026,
 	MCU_SDK_ERRCODE_SOAP_UDP_ERROR								=64027,
 	MCU_SDK_ERRCODE_SOAP_TCP_ERROR								=64028,
 	MCU_SDK_ERRCODE_SOAP_HTTP_ERROR								=64029,
 	MCU_SDK_ERRCODE_SOAP_SSL_ERROR								=64030,
 	MCU_SDK_ERRCODE_SOAP_ZLIB_ERROR								=64031,
 	MCU_SDK_ERRCODE_SOAP_DIME_ERROR								=64032,
 	MCU_SDK_ERRCODE_SOAP_DIME_HREF								=64033,
 	MCU_SDK_ERRCODE_SOAP_DIME_MISMATCH							=64034,
 	MCU_SDK_ERRCODE_SOAP_DIME_END								=64035,
 	MCU_SDK_ERRCODE_SOAP_MIME_ERROR								=64036,
 	MCU_SDK_ERRCODE_SOAP_MIME_HREF								=64037,
 	MCU_SDK_ERRCODE_SOAP_MIME_END								=64038,
 	MCU_SDK_ERRCODE_SOAP_VERSIONMISMATCH						=64039,
 	MCU_SDK_ERRCODE_SOAP_PLUGIN_ERROR							=64040,
 	MCU_SDK_ERRCODE_SOAP_DATAENCODINGUNKNOWN					=64041,
 	MCU_SDK_ERRCODE_SOAP_REQUIRED								=64042,
 	MCU_SDK_ERRCODE_SOAP_PROHIBITED								=64043,
 	MCU_SDK_ERRCODE_SOAP_OCCURS									=64044,
 	MCU_SDK_ERRCODE_SOAP_LENGTH									=64045,
 	MCU_SDK_ERRCODE_SOAP_FD_EXCEEDED							=64046,
	MCU_SDK_ERRCODE_SOAP_RECV_TIME_OUT							=64050,		//SOAP RECV MESSAGE TIME OUT
	
	//other
	OCX_INIT_ERR 												=66000,//初始化错误
	OCX_WAIT_REC_OVERTIME										=66001,//录像查询等待结束标志超时
	OCX_UNINIT_ERR												=66002,//反初始化错误
};

enum EDecoderMod
{
	emDecoderModUnable = 0,
	emBaseDec = 1,
};
	
enum EStreamMod
{
	emStreamModUnable = 0,
	emPlat1 = 1,
	emPlat2 = 2,
	emG900 = 3,
};
	
enum EBussinessMod
{
	emBussinessModUnable = 0,
	emPlat1BS = 1,
	emPlat2BS = 2,
};

enum eLogLevel
{
    ForbidFlow = 0,//Forbid all flow
    NomalFlow = 1, // normal flow
    KeyFlow =2, // key flow
    SuccFlow = 3,//Succ flow must 
    FailFlow = 4,//Fail flow must
    
};

enum EWatchTYPE
{
	emWatchInVailed = 0,
	emWatchTask = 1,
	emWatchPrePos =2,
};


// set string null, pStr: string pointer dwStrLen: string len( the length not include end '\0' )
inline void SetStringNull(s8 *pStr, u32 dwStrLen)
{
	for (int nNoIdx = 0; nNoIdx <= dwStrLen; nNoIdx++) {
        pStr[nNoIdx] = '\0';
    }
}

inline void StringCopy(char *pStrDst, char *pStrSrc, u32 dwStrLen)
{
    if( pStrDst == NULL || pStrSrc == NULL )
    {
        return;
    }
    for (int nNoIdx = 0; nNoIdx < dwStrLen; nNoIdx++) {
        pStrDst[nNoIdx] = pStrSrc[nNoIdx];
    }
}

#pragma pack(1)

/* KDMNO define */
struct KDMNO
{
    KDMNO( void );
    void SetNull();
    bool IsNull() const;
    void operator = ( const KDMNO &tObj );
    bool operator == ( const KDMNO &tObj ) const;
	bool operator != ( const KDMNO &tObj ) const;
    bool operator < ( const KDMNO &tObj ) const;
    
    char szID[MAX_ID_LEN+1];
};

inline void KDMNO::SetNull()
{
	SetStringNull(szID, MAX_ID_LEN);
}

inline KDMNO::KDMNO( void )
{
	SetNull();
}

inline bool KDMNO::IsNull() const
{
	for (int nNoIdx = 0; nNoIdx <= MAX_ID_LEN; nNoIdx++) {
        if( szID[nNoIdx] != '\0' )
        {
            return false;
        }
    }
	return true;
}

inline void KDMNO::operator = ( const KDMNO &tObj )
{
    for (int nNoIdx = 0; nNoIdx <= MAX_ID_LEN; nNoIdx++) {
        szID[nNoIdx] = tObj.szID[nNoIdx];
    }
}

inline bool KDMNO::operator == ( const KDMNO &tObj ) const
{
    for (int nNoIdx = 0; nNoIdx <= MAX_ID_LEN; nNoIdx++) {
        if( szID[nNoIdx] != tObj.szID[nNoIdx] )
        {
            return false;
        }
    }
    
    return true;
}

inline bool KDMNO::operator != ( const KDMNO &tObj ) const
{
    for (int nNoIdx = 0; nNoIdx <= MAX_ID_LEN; nNoIdx++) {
        if( szID[nNoIdx] != tObj.szID[nNoIdx] )
        {
            return true;
        }
    }
    
    return false;
}

inline bool KDMNO::operator < ( const KDMNO &tObj ) const
{
    return ( strcmp(szID, tObj.szID) > 0 );
}

/* bussiness define */
// define id
typedef KDMNO GroupID;
typedef KDMNO DeviceID;
typedef KDMNO DomainID;
typedef KDMNO DeviceKDMNO;

enum EStreamFlowPattern
{
	emStreamPatenTCP = 0,
	emStreamPatenUDP = 1,
	emStreamPatenPIC = 2,
};

// subscript type
enum ESubscriptInfo
{
    emOnline = 0x01,
    emAlarm = 0x02,
    emVidChn = 0x04,
    emDevState = emOnline | emAlarm | emVidChn,
    emGPSInfo = 0x08,
    emAllSub =  emDevState|emGPSInfo,
    emTvWallState	=0x100,
};

enum EAudioEncType
{
	emInvailed = 0,
	emg711 = 1,
	enaaclc = 2,
	emadpcm = 3,
};

//磁盘满覆盖策略
enum emCoverPolicy
{
	emSDK_RS_COVERPOLICY_INVALID = 0,
	emSDK_RS_COVERPOLICY_STOPRECORD = 1,          /*通道空间满停录像*/
	emSDK_RS_COVERPOLICY_COVERALL= 2,            /*通道空间满覆盖所有录像*/
	emSDK_RS_COVERPOLICY_COVERNORMAL = 3,         /*通道空间满覆盖普通录像*/
	emSDK_RS_COVERPOLICY_UNKNOWN = 4,
};

// device group infomation struct
typedef struct DeviceGroupInfo
{
    DeviceGroupInfo()
    {
        SetStringNull(szGroupName, MAX_NAME_LEN+1);
        bHasDevice = 0;
	 groupID.SetNull();
	 parentID.SetNull();
    }
    void Copy(const DeviceGroupInfo &tObj);
    void operator = ( const DeviceGroupInfo &tObj );
    GroupID groupID; // group id, from plat
    GroupID parentID; // parent group id, from plat
    char szGroupName[MAX_NAME_LEN+1]; // group name, from plat
    bool bHasDevice; // is the group has device, from plat
} GROUPINFO;

inline void DeviceGroupInfo::operator = ( const DeviceGroupInfo &tObj )
{
    Copy(tObj);
}

inline void DeviceGroupInfo::Copy(const DeviceGroupInfo &tObj)
{
    groupID = tObj.groupID;
    parentID = tObj.parentID;
    StringCopy((char*)szGroupName, (char*)tObj.szGroupName, sizeof(szGroupName));
    bHasDevice = tObj.bHasDevice;
}

// device information
// device source
typedef struct VideoSrcInfo
{
    VideoSrcInfo()
    {
        nSn = 0;
        SetStringNull(szSrcName, sizeof(szSrcName));
        nPtzLevel = 0;
	 	bFitCondition = FALSE;	
	  	bChanIsOnline = 0X0000;
    }
    void operator = ( const VideoSrcInfo &tObj );
    int nSn; // Video Source id(No.)
    char szSrcName[MAX_NAME_LEN+1]; // video source alias
    int nPtzLevel; // video source ptz level
    BOOL32	bFitCondition;// when search Fit ,show
    BOOL32     bChanIsOnline;
    void 	SetReChnNameBySDK()
    {
		bChanIsOnline = 1;
    }
    void SetRcChnNameByPlatform()
    {
		bChanIsOnline = 0;
    }

    BOOL32 ChnReNamebyPlatform()
    {
		if((bChanIsOnline&0x0001)==1)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
    }
    	
} VIDSRC;

inline void VideoSrcInfo::operator = ( const VideoSrcInfo &tObj )
{
    nSn = tObj.nSn;
    nPtzLevel = tObj.nPtzLevel;
    //StringCopy((char*)szSrcName, (char*)tObj.szSrcName, sizeof(szSrcName));
	StringCopy((char*)szSrcName, (char*)tObj.szSrcName, MAX_NAME_LEN);
}

typedef struct DeviceInfo
{
    enum eDeviceCap // device capability
    {
        emBullet=0,
		emDome,
	};
	enum eDeviceType // device type
    {
        emTypeUnknown = 0,
        emTypeEncoder,
        emTypeDecoder,
        emTypeCodecer,
        emTypeTVWall,
        emTypeNVR,
        emTypeSVR,
        emAlarmHost,
    };
    DeviceInfo()
    {
        nSn = 0;
        nDevSrcNum = 0;
        nEncoderChnNum = 0;
        //SetStringNull(szDevSrcAlias, MAX_NAME_LEN);
		SetStringNull(szDevSrcAlias, sizeof(szDevSrcAlias));
        nDevType = emTypeUnknown;
        nDevCap = emBullet;
        nCallType = 0;
	 	bDvcIsOnline = FALSE;
        SetStringNull(szManufacturer, MAX_NAME_LEN);
	  	szParentGroupID.SetNull();
	  	deviceID.SetNull();
	  	domainID.SetNull();
    }
    void operator = ( const DeviceInfo &tObj );
    int nSn; // device sn, create by mcusdk (nouse in this version)
    DeviceID deviceID; // device id, from plat
    DomainID domainID; // domain id, from plat
    u16 nDevSrcNum; // device video source number
    u16 nEncoderChnNum; // device encoder channel number
    char szDevSrcAlias[MAX_NAME_LEN+1]; // device alias
    u16 nDevType; // device type
    u16 nDevCap; // device capability
    u16 nCallType; // device call type
    VIDSRC aDevSrcChn[MAX_VID_SRC_NUM]; // device video source infomation
    char szManufacturer[MAX_NAME_LEN+1]; // device manufacturer

    //
    BOOL32 bDvcIsOnline;
    GroupID szParentGroupID;
} DEVICEINFO;

typedef struct DevGPSInfo
{
	DevGPSInfo()
	{
		revTime = 0;
		longitude = 0;
		latitude = 0;
		marLongitude = 0;
		marLatitude = 0;
		speed = 0;
		deviceID.SetNull();
	}
	u32 revTime;
	DeviceID deviceID; 
	double longitude;	
	double latitude;	
	double marLongitude;	
	double marLatitude;	
	double speed;	 
	
}DeviceGPSInfo;

typedef struct tagDevLatiLongTude
{
	tagDevLatiLongTude()
	{
		longitude = 0;
		latitude = 0;
		DevSrcId = 0;
		DevChnId = 0;
	}
	DeviceID deviceID; 
	int DevSrcId;
	int DevChnId;
	double longitude;	
	double latitude;	
}TDevLatiLongTude;
    
typedef struct tagDeviceSrcStatus
{
    tagDeviceSrcStatus()
    {
        bEnable = FALSE;
        bOnline = FALSE;
    }
    BOOL32 bEnable;
    BOOL32 bOnline;
} DEVSRC_ST;

typedef struct SearchDvcRspInfo
{
	SearchDvcRspInfo()
	{
		m_DvcInfoType = 0;
        SetStringNull(m_DevDirectory, MAX_DEV_DIR);
	}

	short 		m_DvcInfoType; 	//1:gruoup,2:devices
	GroupID 	m_ParentID;
	char		m_DevDirectory[MAX_DEV_DIR];
	
	DEVICEINFO m_DevInfo;
	GROUPINFO  m_GroupInfo;
	
}StSearchDvcRspInfo;

typedef struct SearchDvcReqInfo
{
	SearchDvcReqInfo()
	{
		m_PBReceiveChnNum = 0;
		m_PBwServerPort = 0;
        SetStringNull(m_PBsearchDvcName, MAX_NAME_LEN);
        SetStringNull(m_PBFeatureCode, MAX_NAME_LEN);
        SetStringNull(m_PBszServerIp, MAX_URL_LEN);
		m_SearchResLimit = 0;
	}
	char       m_PBsearchDvcName[MAX_NAME_LEN];
	char   	   m_PBFeatureCode[MAX_NAME_LEN];
	char       m_PBszServerIp[MAX_URL_LEN];// connect stream server ip(no use now)
	int 	   m_PBReceiveChnNum;	
	int        m_PBwServerPort;				 // connect stream server port(no use now)
	int 	   m_SearchResLimit;

}StSearchDvcReqInfo;

typedef struct SearchDvcRsp
{
	u32  		m_TotalSearchChannelNum;
	u32			m_HasGiveChannelNum;
	u32			m_CurrentGiveDvcNum;
	std::vector <SearchDvcRspInfo> m_vctSearchDvcRspList;
}StSearchDvcRsp;


inline void DeviceInfo::operator = ( const DeviceInfo &tObj )
{
    nSn = tObj.nSn;
    deviceID = tObj.deviceID;
    szParentGroupID = tObj.szParentGroupID;
    domainID = tObj.domainID;
    nDevSrcNum = tObj.nDevSrcNum;
    nEncoderChnNum = tObj.nEncoderChnNum;
    //StringCopy( (char*)szDevSrcAlias, (char*)tObj.szDevSrcAlias, sizeof(szDevSrcAlias) );
	StringCopy( (char*)szDevSrcAlias, (char*)tObj.szDevSrcAlias, MAX_NAME_LEN );
    nDevType = tObj.nDevType;
    nDevCap = tObj.nDevCap;
    nCallType = tObj.nCallType;
    for( int nSrcNum = 0; nSrcNum < MAX_VID_SRC_NUM; nSrcNum++ )
    {
        aDevSrcChn[nSrcNum] = tObj.aDevSrcChn[nSrcNum];
    }
    StringCopy( (char*)szManufacturer, (char*)tObj.szManufacturer, sizeof(szManufacturer) );
}


// device channel
#define INVALID_SRC_INDEX (0xFFFF)
#define INVALID_CHN_INDEX (0x00FF)
enum eChnType
{
    emMaster = 0,
    emSlaver = 1,
    emThird = 2,
    emFourth = 3,
};

// the struct used for function param in usual
typedef struct Device_Chn
{
    Device_Chn()
    {
        nSrc = INVALID_SRC_INDEX;
        nChn = INVALID_CHN_INDEX;
    }
    u16 GetSourceID()
    {
        return nSrc;
    }
    eChnType GetChannelType()
    {
        switch( nChn>>16 )
        {
            case (u32)emMaster:
                return emMaster;
            case (u32)emSlaver:
                return emSlaver;
            case (u32)emThird:
                return emThird;
            case (u32)emFourth:
                return emFourth;
        }
        return emMaster;
    }
    void SetSourceID(u16 wSrcId)
    {
        nSrc = wSrcId;
    }
    void SetIsSlaverChn(BOOL32 bSlaver)
    {
        if( bSlaver )
        {
            nChn |= (((u32)emSlaver)<<16);
        }
        else
        {
            nChn |= (((u32)emMaster)<<16);
        }
    }
    void operator = ( const Device_Chn &tObj );
    DomainID domainID; // device domain id
    DeviceID deviceID; // device id
    u16 nSrc; // video source No.
    u16 nChn; // encoder channel No.
} DEVCHN;

inline void Device_Chn::operator = ( const Device_Chn &tObj )
{
    domainID = tObj.domainID;
    deviceID = tObj.deviceID;
    nSrc = tObj.nSrc;
    nChn = tObj.nChn;
}

// device status notify, use in callback function
enum eStatusType
{
    emDevOnline = 0,
    emDevAlarm = 1,
    emDevConfig = 2,
    emDevGpsInfo = 3,
	emTvWallNewNtf=10,
	emTvWallDelNtf = 11,
	emTvWallModNtf = 12,
    emTvWallStaChgNtf = 13,
};
    
enum eAlarmType
{
    emAlarmTypeInvalid = 0,
    emAlarmMove = 1,
    emAlarmInput = 2,
    emAlarmDiskfull = 3,
    emAlarmVideoLost = 4,
    emAlarmIntelligent = 5,
    emAlarmVideo = 6,
};

enum eStreamUrlType
{
	emInvailedUrlType = 0,
	emUrlTypeTCPFlow = 1,
	emUrlTypeUDPFlow = 2,
	emUrlTypePICFlow = 3,
};
    
enum eAlarmStatus
{
    emAlarmOccur = 1,
    emAlarmResume = 2,
};

typedef struct DeviceStatus
{
    DeviceStatus()
    {
        memset(this, 0x00, sizeof(DeviceStatus));
    }
    eStatusType m_emStatusType; // device status type
    union
    {
        struct {
            u32 m_dwAlarmType; // alarm type
            u8 m_byAlarmStatus; // alarm status: Occur, resume
            u32 m_dwTime; // alarm Occur or Resume time
            u16 m_wAlarmChn; // alarm channel
        } m_AlarmParam; // alarm info
        BOOL32 m_bOnline; // device online: 1 offline: 0
    };
} DEVSTATUS;

// subscript device array
typedef struct tagSubsDevices
{
    tagSubsDevices()
    {
        m_bySubsDevNum = 0;
        memset( m_vctDevID, 0x00, MAX_REQ_DEV_SUBS_NUM*sizeof(DeviceID) );
    }
    BOOL32 AddDevice(DeviceID tDevice)
    {
        if( m_bySubsDevNum < MAX_REQ_DEV_SUBS_NUM )
        {
            m_vctDevID[m_bySubsDevNum++] = tDevice;
            return true;
        }
        return false;
    }
    DeviceID GetDevice(u8 byIndex)
    {
        if( byIndex >= MAX_REQ_DEV_SUBS_NUM )
        {
            return DeviceID();
        }
        return m_vctDevID[byIndex];
    }
    u8 m_bySubsDevNum; // subscript device number
    DeviceID m_vctDevID[MAX_REQ_DEV_SUBS_NUM]; // subscript device id
} TSUBSDEVS;

// device status callback, (after subscript device, mcusdk will send status on application by this function)
typedef int ( *DevStatus_Callback)(DeviceID devID, DeviceStatus *pDevStatus, u32 lUserData);

//modified by yuhengyue 2014.9.17
//增加左上，左下，右上，右下PTZ枚举量
enum ePtzCmd
{
    emMoveLeft = 0,
    emMoveRight = 1,
    emMoveUp = 2,
    emMoveDown = 3,
	emMoveLeftUp = 4,
	emMoveLeftDown = 5,
	emMoveRightUp = 6,
	emMoveRightDown = 7,
    emMoveStop = 8,
    emZoomIn = 9,
    emZoomOut = 10,
    emZoomStop = 11,
    emHome = 12,
	emAutoScanStart =128,  // 128	//开始自动巡航
	emAutoScanStop = 129,	   // 129	//停止自动巡航	
};

// ptz command info
typedef struct tagPtzCmd
{
    tagPtzCmd()
    {
        m_emPtzCmd = 0;
        m_byLevel = 0;
        m_dwHoldTimer = 0;
        m_wPtzRange = 0;
    }
    int m_emPtzCmd; // ptz command
    u16 m_wPtzRange; // ptz range
    u8 m_byLevel; // device source ptz level (option)
    u32 m_dwHoldTimer; // ptz hold time (option)
} PTZCMD;

typedef struct tagPtzAuthority
{
	tagPtzAuthority()
	{
		m_wPtzRange = 0;
		m_byLevel = 0;
		m_dwHoldTimer = 0;
	}
	u16 m_wPtzRange;//ptz ranger
    u8 m_byLevel; // device source ptz level (option)
    u32 m_dwHoldTimer; // ptz hold time (option)
}TPTZAUTHORITY;

// pu point move
typedef struct tagPointMoveCmd
{
    tagPointMoveCmd()
    {
        m_wPosX = 0;
        m_wPosY = 0;
        m_wScreenWidth = 0;
        m_wScreenHeight = 0;
    }
    u16 m_wPosX; // position x coordinate in screen
    u16 m_wPosY; // position y coordinate in screen
    u16 m_wScreenWidth; // screen width
    u16 m_wScreenHeight; // screen height
} POINTMOVE;

typedef struct tagDrawWnd
{
	union
	{
		struct
		{
		    void *jvm;
			void *surfaceView;
		}tAndroidWnd;
		struct
		{
			int channel;
			int windowwidth;
			int windowheight;
		}tIosWnd;
	};
}TDrawWnd,*PTDrawWnd;

/* stream define */
// play start trans mode
enum eTransMode
{
    Trans_UNKNOW,
    Trans_UDP,
    Trans_TCP,
    Trans_KWTP,
};

// play media type
enum eMediaType
{
    Media_UNKNOW,
    Media_VIDEO,
    Media_AUDIO,
    Media_ALL,
};

// play snapshot type
enum EPictureType
{
    emBmp32 = 0,
    emJpeg_100 = 1,
    emJpeg_70 = 2,
    emJpeg_50 = 3,
    emJpeg_30 = 4,
    emJpeg_10 = 5,
    emBmp24 = 6,
};

// play local record type
enum ELocalRecType
{
    emMp4 = 0,
    em3gp = 1,
    emAsf = 2,
};

enum ePlayVideoType
{   
	emInvailedVideoPlay =-1,
	emRecordVideoPlay = 0,
	emRealVideoPlay = 1,
	emRecordVideoDownLoad = 2,
	emRecordPlayBySETime = 3,
};

enum eRecordType
{   
	emInvailedRecord = 0,
	emPlatFormRecord = 1,
	emIpcRecord = 2,
	emLocalRecord = 4,
};


// stream parameter
typedef struct Stream_Param
{
    Stream_Param()
    {
        SetStringNull(m_szServerIp, MAX_URL_LEN);
        SetStringNull(m_szManufactor, MAX_NAME_LEN);
        m_wServerPort   = 0;
        m_wScreenWidth  = 0;
        m_wScreenHeight = 0;
        //Add by xql 20140825
        m_wHighDefinition = FALSE;
        m_playTime  = 0;
        m_startTime =0;
        m_endTime   =0;
        m_emVideoType  = emInvailedVideoPlay;
        m_byRecordType = emInvailedRecord;
        //end
        m_dwDownloadStartTime = 0;
        m_dwDownloadEndTime   = 0;
        m_dwDownloadFirstPlayTime = 0;
    }
	s8  m_szServerIp[MAX_URL_LEN+1];// connect stream server ip(no use now)
	u16 m_wServerPort;				 // connect stream server port(no use now)
	
    u16 m_wScreenWidth;				 // play screen width
    u16 m_wScreenHeight;				 // play screen height
    
    void* m_pDrawWnd; // play window handle
    s8    m_szManufactor[MAX_NAME_LEN+1]; // device manufactor
    //Add by xql 20140825
    BOOL32 m_wHighDefinition;
    //end
	
    //add by xql 20141008
	ePlayVideoType	m_emVideoType;
    u32	m_startTime;
    u32	m_endTime;
    u32 m_playTime;
	eRecordType	m_byRecordType;
    //end
    u32 m_dwDownloadStartTime;
    u32 m_dwDownloadEndTime;
    u32 m_dwDownloadFirstPlayTime; //用于跨文件计算下载进度
} SPARAM;

//recv stream type
enum eRecvStreamType
{
    VIDEO_STREAM_TYPE = 1, // video type
    AUDIO_STREAM_TYPE = 2, // audio type
};

typedef struct tagFrameHdr_SDK
{
    u8     m_byMediaType;
    u8    *m_pData;
    unsigned int m_dwPreBufSize;
    unsigned int    m_dwDataSize;
    u8     m_byFrameRate;
    unsigned int    m_dwFrameID;
    unsigned int    m_dwTimeStamp;
    unsigned int    m_dwSSRC;
    union
    {
        struct{
			BOOL32    m_bKeyFrame;
			u16       m_wVideoWidth;
			u16       m_wVideoHeight;
		}m_tVideoParam;
        u8    m_byAudioMode;
    };
	
}FRAMEHDR_SDK,*PFRAMEHDR_SDK;

/*typedef struct tagFrameYUV
{
    u8 *m_pData; // YUV data
    u32 m_dwDataSize; // YUV data length
    u32 m_dwWidth; // image width
    u32 m_dwHeight; // image height
    u32 m_dwTimeStamp; // frame time stamp
} PFRAMEYUV;*/

//该tagFrameYUV结构体与uniplay里KDFrameInfo结构体完全相同！
typedef struct tagFrameYUV
{
	u8*					pbyRawBuf;
	unsigned int		dwRawlen;
	unsigned int		dwMediaType;			//数据类型(KD_FRAME_TYPE)
	unsigned int		dwSubMediaType;			//KD_DATA_TYPE(视频有效)
	unsigned int		dwFrameIndex;			//帧序号
	u64					dwNetTimeStamp;			//相对时间戳（RTP内部时间戳）
	u64					dwMediaTimeStamp;		//绝对时间戳（视频上面时间）
	unsigned int		dwMediaEncode;			//在kdvdef.h 内部定义或者FRAME_DATA_FORMAT里面定义
	union FRAME
	{
		struct VideoFrame
		{
			unsigned short		dwFrameRate;
			unsigned short		dwWidth;     //画面宽
			unsigned short		dwHeight;    //画面高
			unsigned short		strike[3];
		}tVideo;
		
		struct AudioFrame
		{
			unsigned short		wChannels;   //声道数
			unsigned short		wBitWidth;   //位宽
			unsigned short		dwSampleRate; //采样率
		}tAudio;
	}u;
	unsigned short		wBitRate;			//比特率，单位为1028字节
	unsigned short		wReverse[3];
}FRAMEYUV_SDK,*PFRAMEYUV_SDK;

// stream url infomation (from stream server)
typedef struct Stream_UrlInfo
{
    Stream_UrlInfo()
    {
        SetStringNull(m_szUrl, MAX_URL_LEN+1);
        SetStringNull(m_szDesc, MAX_DESC_LEN+1);
	 	SetStringNull(m_manuFactory, MAX_DESC_LEN+1);
        SetStringNull(m_reserve, MAX_DESC_LEN+1);
	 	m_dwWidth = 0;
	 	m_dwHeight = 0;
		m_UrlType = emInvailedUrlType;
    }
	s8	   m_szUrl[MAX_URL_LEN+1]; 		  // stream url
	u32    m_dwWidth; 					  // stream width
	u32    m_dwHeight; 					  // stream height
	s8	   m_szDesc[MAX_DESC_LEN+1]; 	  // stream description
    s8     m_manuFactory[MAX_DESC_LEN+1]; // ManuFactory
    s8     m_reserve[MAX_DESC_LEN+1];	  // Reserve
    eStreamUrlType	   m_UrlType;				 	  // Url的类型 0:不合法的方式;1:TCP方式,从MVS获取码流;2:UDP方式，从平台获取视频码流;3:图片流
} URLINFO;

typedef struct App_Version_Url
{
	App_Version_Url()
	{
        SetStringNull(m_appVersion, MAX_URL_LEN+1);
        SetStringNull(m_appUpdateUrl, MAX_URL_LEN+1);
	}
	s8 m_appVersion[MAX_URL_LEN+1];
	s8 m_appUpdateUrl[MAX_URL_LEN+1];
}APPVERSIONURL;


// stream url list
typedef struct Stream_UrlList
{
    Stream_UrlList()
    {
        m_byUrlNum = 0;
    }
    u8 m_byUrlNum; // url number
    URLINFO m_aStreamUrl[MAX_STREAM_URL_NUM]; // stream url info
} URLLIST;

// stream callback dwPlayId: stream play id(from StartRealPlay) pFrmHdr: stream data dwContext: user data
typedef void ( *Stream_Callback)(u32 dwPlayID, PFRAMEHDR_SDK pFrmHdr, u32 dwContext);

// stream YUV callback dwPlayID: stream play id(from StartRealPlay) pFrmYuv: YUV data dwContext: user data
typedef void ( *StreamYuv_Callback)(u32 dwPlayID, PFRAMEYUV_SDK pFrmYuv, void *dwContext);

// select streasm url callback
typedef int ( *SelectStreamUrl_Callback) (const URLLIST *pUrlList, u32 lUserData);

// stream callback
typedef struct SDKCallbackMedia
{
    SDKCallbackMedia()
    {
        m_pSDKFrameCB = NULL;
        m_pSDKYUVCB = NULL;
        m_pSDKUrlCB = NULL;
        m_dwSDKUserData = 0;
    }
    Stream_Callback m_pSDKFrameCB; // stream frame callback
    StreamYuv_Callback m_pSDKYUVCB; // stream yuv data callback,after decode
    SelectStreamUrl_Callback m_pSDKUrlCB; // select stream url callback,for chose an accord url in application
    u32 m_dwSDKUserData; // user data
} CB_SDKMEDIA;

// stream callback (use in mcu sdk)
typedef struct StreamCallback
{
    StreamCallback()
    {
        m_pStreamFrameCB = NULL;
        m_pH264StreamCbFunc = NULL;
        m_pStreamDecodeCB = NULL;
        m_pStreamUrlCB = NULL;
        m_ptStreamSDK = NULL;
        m_dStreamUserData = 0;
    }
    void *m_pStreamFrameCB;//sdk layer H264 CallBack
    void *m_pH264StreamCbFunc;//application layer H264 CallBack
    void *m_pStreamDecodeCB;
    void *m_pStreamUrlCB;
    void *m_ptStreamSDK;
    u32   m_dStreamUserData;
} CB_STREAM;

/* error info(error to application) */
// work type(mcusdk work)
enum eWorkType
{
    emLogin = 1,
    emLogout =2,
    emGetGroup=3,
    emGetDevic=4,
    emStartStream=5,
    emRecvKeyFrame=6,
    emStopStream=7,
    emPtz=8,
    emSubscript=9,
    emLocalRec=10,
    emSnapshot=11,
    emSearchRec=12,
    emStartPlayRec=13,
    emStopPlayRec=14,
    emVcrCtrl=15,
    emHeartbeatError=16,
    emSearchDvc=17,
    emRecordDownLoad=18,
    emRecordDownLoadPace=19,
	emSyncTime = 20,
    emUnknown,
};

// event infomation
typedef struct tagEventInfo
{
    tagEventInfo()
    {
        m_dwErrorCode = 0;
        m_emWork = emUnknown;
        m_dwWorkID = 0;
        m_dwReserve1 = 0;
        m_dwReserve2 = 0;
    }
    u32 m_dwErrorCode; // error code
    eWorkType m_emWork; // work
    u32 m_dwWorkID; // work id: playid/taskid...
    u32 m_dwReserve1;
    u32 m_dwReserve2;
} EVENTINFO;

// error callback, (mcusdk will call this fucntion when error occur)
typedef int ( *McuSdkEvent_Callback)(EVENTINFO *pEventInfo, u32 lUserData);

//Time Period
typedef struct tagTimePeriod
{
    tagTimePeriod()
    {
        m_dwStartTime = 0;
        m_dwEndTime = 0;
    }
    u32 m_dwStartTime;
    u32 m_dwEndTime;
} TPeriod;

typedef struct tagRecordInfo
{
    tagRecordInfo()
    {
        memset(m_dwRecID,0,sizeof(m_dwRecID));
        memset(m_recordDomainName,0,sizeof(m_recordDomainName));
        m_eRecType = emInvailedRecord;
    }
    TPeriod m_tRecPeriod;
    char 	m_dwRecID[MAX_DEV_DIR+1];
    char    m_recordDomainName[MAX_DEV_DIR+1];
    DEVCHN 	m_tDevChn;
    eRecordType m_eRecType;
} TRecordInfo;

typedef struct tagRecordDownloadInfo
{
	tagRecordDownloadInfo()
	{
		//m_pFileFullName = NULL;
		m_emFileType = emMp4;
		m_byRecordType = emInvailedRecord;
		memset(m_pFileFullName,0,sizeof(m_pFileFullName));
	}
	TPeriod m_tRecPeriod;
	//char* m_pFileFullName;
	char m_pFileFullName[MAX_SM_STR_LEN+1];
	ELocalRecType m_emFileType;
	eRecordType	m_byRecordType;
	u32 m_dwReserve1;
    u32 m_dwReserve2;
}TRecordDownloadInfo;

typedef struct tagRecordPlayInfo
{
	tagRecordPlayInfo()
	{
		m_byRecordType = emInvailedRecord;
		m_pDrawWnd = NULL;
	}
	TPeriod m_tRecPeriod;
	eRecordType	m_byRecordType;
	void* m_pDrawWnd;
    s8    m_szManufactor[MAX_NAME_LEN+1]; // device manufactor
	u32 m_dwReserve1;
    u32 m_dwReserve2;
}TRecordPlayInfo;

#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef struct tagVidOSD
{
	char m_abyContent[MAX_DISPLAY_CONTENT_LEN + 1]; //字幕内容，使用表示换行
	u16 m_wLeftMargin;
	u16 m_wTopMargin;
	tagVidOSD()
	{
		memset(this, 0, sizeof(tagVidOSD));
	}
}TVidOSD;
enum RecordPlayCtrlType
{
    CTRL_Invailed = -1,
    CTRL_PLAY = 0,
    CTRL_PAUSE = 1,
    CTRL_SEEK = 2,
    CTRL_SCALE = 3,
};

typedef struct tagRecordPlayCtrlInfo
{
	tagRecordPlayCtrlInfo()
	{	
		m_byCtrlType =(RecordPlayCtrlType)-1;
		m_dwRange = 0;
		m_dwPlayId = -1;
	}
	
	char m_byCtrlType; // ??Χmediasdk_CtrlType
	int m_dwRange; 				// playseek????, scale????????
	unsigned int m_dwPlayId;					//dwplayID
}TREPCTLINFO;

typedef struct tagEncoderParam
{
    tagEncoderParam()
    {
        memset(this, 0, sizeof(tagEncoderParam));
    }
    u8 m_byVideoFormat;
    u32 m_dwVideoResolution;
    u8 m_byFrameRate;
    u8 m_byQuality;
    u32 m_dwBitRate;
    u16 m_wInterval;
    u8 m_byBrightness;
    u8 m_byContrast;
    u8 m_bySaturation;
    u16 m_wSharpness;
} TEncoderParam;

enum eVideoFormat
{
    em_VF_VideoInvalid = 0,
    em_VF_SN4 = 1,
    em_VF_MPEG4,
    em_VF_H261,
    em_VF_H263,
    em_VF_H264,
};

enum eVideoResolution
{
    em_VR_ResInvalid = 0,
    em_VR_Auto = 0x0001,
    em_VR_Qcif = 0x0002,
    em_VR_Cif = 0x0004,
    em_VR_2Cif = 0x0008,
    em_VR_4Cif = 0x0010,
    em_VR_QQcif = 0x0020,
    em_VR_QVGA = 0x0040,
    em_VR_VGA = 0x0080,
    em_VR_720P = 0x0100,
    em_VR_1080P = 0x0200,
    em_VR_QXGA = 0x0400,
};

enum eVideoQuality
{
    em_VP_Invalid = 0,
    em_VP_Quality = 1,
    em_VP_Speed = 2,
};

typedef struct tagSupportResolution
{
    tagSupportResolution()
    {
        m_byVideoFormat = em_VF_VideoInvalid;
        m_dwSupportResolution = em_VR_ResInvalid;
    }
    void SetSupportResolution(u32 dwSupportResolution)
    {
        m_dwSupportResolution = dwSupportResolution;
    }
    void AppendSupportResolution(u32 dwSupportResolution)
    {
        m_dwSupportResolution |= dwSupportResolution;
    }
    u32 GetSupportResolution()
    {
        return m_dwSupportResolution;
    }
    BOOL32 IsSupportResolution(eVideoResolution emVidRes)
    {
        return m_dwSupportResolution & emVidRes;
    }
    void SetVideoMode(eVideoFormat emVideoFormat)
    {
        m_byVideoFormat = emVideoFormat;
    }
    void SetVideoMode(u8 byVideoFormat)
    {
        m_byVideoFormat = byVideoFormat;
    }
    eVideoFormat GetVideoFormat()
    {
        eVideoFormat emRtnFormat = em_VF_VideoInvalid;
        switch( m_byVideoFormat )
        {
            case 1:
                emRtnFormat = em_VF_SN4;
                break;
            case 2:
                emRtnFormat = em_VF_MPEG4;
                break;
            case 3:
                emRtnFormat = em_VF_H261;
                break;
            case 4:
                emRtnFormat = em_VF_H263;
                break;
            case 5:
                emRtnFormat = em_VF_H264;
                break;
        }
        return emRtnFormat;
    }
    u8 m_byVideoFormat;
    u32 m_dwSupportResolution;
} TSupportResolution;

typedef struct tagEncoderParamTotal
{
    tagEncoderParamTotal()
    {
        
    }
    BOOL32 IsSupportVideoFormat(eVideoFormat emVidFormat)
    {
        for( u8 byIdx = 0; byIdx < MAX_SUPPORT_VIDEO_TYPE_NUM; byIdx++ )
        {
            if( m_atSpResolution[byIdx].GetVideoFormat() == em_VF_VideoInvalid )
            {
                return FALSE;
            }
            if( m_atSpResolution[byIdx].GetVideoFormat() == emVidFormat )
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    void SetSupportVideoResolution(eVideoFormat emVidFormat, u32 dwSPResolution)
    {
        for( u8 byIdx = 0; byIdx < MAX_SUPPORT_VIDEO_TYPE_NUM; byIdx++ )
        {
            if( m_atSpResolution[byIdx].GetVideoFormat() == em_VF_VideoInvalid ||
                m_atSpResolution[byIdx].GetVideoFormat() == emVidFormat )
            {
                m_atSpResolution[byIdx].SetVideoMode(emVidFormat);
                m_atSpResolution[byIdx].SetSupportResolution(dwSPResolution);
                break;
            }
        }
    }
    void AppendSupportVideoResolution(u8 byVidFormat, u32 dwSPResolution)
    {
        for( u8 byIdx = 0; byIdx < MAX_SUPPORT_VIDEO_TYPE_NUM; byIdx++ )
        {
            if( m_atSpResolution[byIdx].GetVideoFormat() == em_VF_VideoInvalid ||
               m_atSpResolution[byIdx].GetVideoFormat() == byVidFormat )
            {
                m_atSpResolution[byIdx].SetVideoMode(byVidFormat);
                m_atSpResolution[byIdx].AppendSupportResolution(dwSPResolution);
                break;
            }
        }
    }
    TEncoderParam m_tEncoderParam;
    TSupportResolution m_atSpResolution[MAX_SUPPORT_VIDEO_TYPE_NUM];
} TEncoderParamTotal;

typedef struct TagPlayExtInfo
{
	TagPlayExtInfo()
	{
		LocalVidRtpPort = 0;
		LocalVidRtcpPort = 0;
		LocalAudRtpPort = 0;
		LocalAudRtcpPort = 0;
		memset(LocalStreamIP,0,sizeof(LocalStreamIP));
		RemoteVidRtpPort = 0;
		RemoteVidRtcpPort = 0;
		RemoteAudRtpPort = 0;
		RemoteAudRtcpPort = 0;
		memset(RemoteStreamIP,0,sizeof(RemoteStreamIP));
		memset(mediaStreamType,0,sizeof(mediaStreamType));
	};
	u16 LocalVidRtpPort;
	u16 LocalVidRtcpPort;
	u16 LocalAudRtpPort;
	u16 LocalAudRtcpPort;
	char LocalStreamIP[MAX_ID_LEN];
	u16 RemoteVidRtpPort;
	u16 RemoteVidRtcpPort;
	u16 RemoteAudRtpPort;
	u16 RemoteAudRtcpPort;
	char RemoteStreamIP[MAX_ID_LEN];
	char mediaStreamType[MAX_ID_LEN];
}PlayExtInfo;

typedef struct tagPlyIngChnAudVidInfo
{
	tagPlyIngChnAudVidInfo()
	{
        //SetStringNull(VideoType, MAX_SM_STR_LEN+1);
        //SetStringNull(AudioType, MAX_SM_STR_LEN+1);
		memset(VideoType,0,sizeof(VideoType));
		memset(AudioType,0,sizeof(AudioType));
		VideoSmpRate = 0;
		AudioSmpRate = 0;
		AudioChnlNum = 0;
	};
	s8	   VideoType[MAX_SM_STR_LEN+1];
	u32	   VideoSmpRate;
	s8	   AudioType[MAX_SM_STR_LEN+1];
	u32    AudioSmpRate;
	u32    AudioChnlNum;

}TTPlyingChnAVInfo;

typedef struct tagTransData
{
    enum eTransType
    {
        emTransChnCOMPort = 0,
        emTransChnCameraTaskConfig = 1,
        emTransChnExtData = 2,
    };
    tagTransData()
    {
		//SetStringNull(m_byTransBuffer, MAX_TRANSCHAN_DATA_LEN+1);
		memset(m_byTransBuffer,0,sizeof(m_byTransBuffer));
        m_wTransBufferLen = 0;
        m_byAppType = 0;
    }
    u8 m_byTransBuffer[MAX_TRANSCHAN_DATA_LEN+1];
    u16 m_wTransBufferLen;
    s8 m_byAppType;
} TTransChnData;


//tvWall Start 
typedef struct tagTvWallCommonData
{
	tagTvWallCommonData()
	{
        //SetStringNull(m_TvWallID, MAX_DESC_LEN+1);
		memset(m_TvWallID,0,sizeof(m_TvWallID));
		m_tvId = 0;
		m_tvDivId = 0;
	}
	char 			m_TvWallID[MAX_DESC_LEN+1];	// 电视墙Id.
	int 			m_tvId;						// 电视机Id.
	int 			m_tvDivId;					// 电视机分画面Id.
}TTvWallCommonData;

typedef struct tagTvWallPlayData
{
	tagTvWallPlayData()
	{
		m_ChnID = 0;
	}
	TTvWallCommonData	m_TvWallComData;
	DeviceID 			m_devURI;				// 编码器/IPC 等视频源设备的唯一标识
	int 				m_ChnID;				// 编码器/IPC 等视频源通道 

}TTvWallPlayData;

typedef struct tagTvDivNumData
{
	tagTvDivNumData()
	{
        //SetStringNull(m_TvWallID, MAX_DESC_LEN+1);
		memset(m_TvWallID,0,sizeof(m_TvWallID));
		m_tvId = 0;
		m_tvDivTotal = 0;
	}
	char 			m_TvWallID[MAX_DESC_LEN+1];	// 电视墙Id.
	int 			m_tvId;						// 电视机Id.
	int 			m_tvDivTotal;				// 电视机画面数
}TTvDivNumData;

typedef struct tagTvWallTotal
{
	tagTvWallTotal()
	{
		tvWallTotal = 0;
		tvWallDecoderBindTotal = 0;
	}
	int tvWallTotal;
	int tvWallDecoderBindTotal;

}TTvWallTotal;

typedef struct tagCTVDecoderBind
{
	tagCTVDecoderBind()
	{
		m_tvId = 0;
		m_decoderOutputId = 0;
		m_tvDivNum = 0;
	}
	int m_tvId;		         // 电视序号
	int m_tvDivNum;			 //电视机画面数
	DeviceID m_decoderId;	 // 解码器设备id.
	int m_decoderOutputId;	// 解码器的输出Id. 

}TTVDecoderBind;


typedef struct tagTvWallInfo
{
	tagTvWallInfo()
	{
		memset(m_TvWallID,0,sizeof(m_TvWallID));
		memset(m_name,0,sizeof(m_name));
		m_tvNum = 0;
		m_tvwallStyle = 0;
		memset(m_tvwallCustomStyleData,0,sizeof(m_tvwallCustomStyleData));
		memset(m_desc,0,sizeof(m_desc));
		m_createTime = 0;
		memset(m_tClientID,0,sizeof(m_tClientID));
		m_tvDecoderBindArrayRealSize = 0;
		m_tvDecoderBindArray =  NULL;
	}
	char 			m_TvWallID[MAX_DESC_LEN+1];				// 电视墙Id.
	DomainID 		m_domainId; 							// 电视墙所属域编号
	char 			m_name[MAX_DESC_LEN+1];    				// 电视墙名称
	int 			m_tvNum;								// 电视机数目
	int 			m_tvwallStyle;							// 电视墙风格, 不使用枚举，因为TAS不关心具体的内容，否则到时风格类型添加时TAS也要改了
	char 			m_tvwallCustomStyleData[MAX_DESC_LEN*10+1];// 电视墙自定义风格数据
	char 			m_desc[MAX_DESC_LEN+1];                 // 电视墙描述
	__time32_t 			m_createTime;		        			// 入网日期
	char			m_tClientID[ADDR_STR_LEN+1];			//客户ID;
	int				m_tvDecoderBindArrayRealSize;
	TTVDecoderBind* m_tvDecoderBindArray; 					// 电视与解码器绑定关系
}TTvWallInfoData;

enum emETVWallDivStyle
{
	emSDK_TVDIV_Invalid = 0, 
	emSDK_TVDIV_1 = 1, 
	emSDK_TVDIV_2 = 2, 
	emSDK_TVDIV_3 = 3, 
	emSDK_TVDIV_4 = 4, 
	emSDK_TVDIV_6 = 6, 
	emSDK_TVDIV_8 = 8, 
	emSDK_TVDIV_9 = 9, 
	emSDK_TVDIV_16 = 16,
	emSDK_TVDIV_25 = 25,
	emSDK_TVDIV_36 = 36,
};

typedef struct tagTPuChn
{
	tagTPuChn()
	{
		m_byEdcChnId = 0;
		m_byVidChnId = 0;
		m_dwSessIDOrRpChn = 0;
		memset(m_achNO,0,sizeof(m_achNO));
	}

    u8  m_byEdcChnId;        //编解码的通道号;
    u8  m_byVidChnId;        //视频源号;
    unsigned int m_dwSessIDOrRpChn;   //会话ID或录放像通道
	char m_achNO[LEN_KDM_NO+1];
}TTPuChn;

typedef struct tagDevChannel
{
	tagDevChannel()
	{
		channelId = 0;
	}
    DeviceID deviceId;     //设备的唯一标识 pu uuid@domain
    int    channelId;      //通道有可能是编码通道，有可能是视频源，还有可能是放像通道, 要根据协议具体应用场景判断
}TDevChannel;

// 窗口轮巡的一步
typedef struct tagTvWallWindowPollStep
{
	tagTvWallWindowPollStep()
	{
		duration = 0;
	}
	TDevChannel		encoderChn;	// 编码器/IPC 等视频源通道.
	int 		duration;					// 该视频源的码流持续时间.
}TTvWallSchemeTvDivPollStep;

// 电视上的单个子窗口(画面合成)的预案.
typedef struct tagTTvWallSchemeTvDiv
{
	tagTTvWallSchemeTvDiv()
	{
		divId = -1;
		tvWallSchTvDivPollStepArrayRealSize = 0;
		tvWallSchTvDivPollStepArray = NULL;
	}
	int divId;	// 解码器通道id.
	TTvWallSchemeTvDivPollStep* tvWallSchTvDivPollStepArray;	// 轮巡的各个步骤. 如果只有一个轮巡, 则是普通浏览.
	int tvWallSchTvDivPollStepArrayRealSize;
}TTvWallSchemeTvDiv;

// 单个电视的预案.
typedef struct tagTvWallTVSchemeTv
{
	tagTvWallTVSchemeTv()
	{
		tvId = -1;
		divStyle = emSDK_TVDIV_Invalid;
		tvWallScheTvDivArray = NULL;
		tvWallScheTvDivArrRealSize = 0;
	}
	int tvId;					// 电视Id
	DeviceID 					m_decoderId;// 解码器设备id.
	enum emETVWallDivStyle divStyle;	// 窗口的画面合成分画面风格.	
	TTvWallSchemeTvDiv* tvWallScheTvDivArray;	// 电视的画面合成的小窗口.	
	int 			  tvWallScheTvDivArrRealSize;
}TTvWallSchemeTv;


// 电视墙的预案.
typedef struct  tagTvWallScheme
{
	tagTvWallScheme()
	{
		memset(schemeName,0,sizeof(schemeName));
		memset(tvWallId,0,sizeof(tvWallId));
		schemeSN = 0;
		memset(m_tClientID,0,sizeof(m_tClientID));
		tvSchemeTvArray = NULL;
		tvSchemeTvArrayRealSize = 0;
	}
	u32  	schemeSN;						//预案编号(创建预案时无需填入)
	char	m_tClientID[ADDR_STR_LEN+1];	//客户ID;
	u8		byOwnerType;					//所属类型
	char 	schemeName[MAX_DESC_LEN+1];	// 预案名称.
	char 	tvWallId[MAX_DESC_LEN+1];	// 预案所属电视墙Id.
	
	TTvWallSchemeTv*  tvSchemeTvArray;	// 电视墙上所有电视的预案列表.	
	int     tvSchemeTvArrayRealSize;
}TTvWallScheme;

typedef struct tagTvWallSchemeTotal
{
	tagTvWallSchemeTotal()
	{
		tvWallSchemeMaxSize = 0;
		tvWallSchemeTvMaxSize= 0;
		tvWallSchemeTvDivMaxSize = 0;
		tvWallSchemeTvDivPollStepMaxSize = 0;
	}
	int tvWallSchemeMaxSize;
	int tvWallSchemeTvMaxSize;
	int tvWallSchemeTvDivMaxSize;
	int tvWallSchemeTvDivPollStepMaxSize;

}TTvWallSchemeTotal;
//tvWall End


//SchemePollStep Start
typedef struct tagSchemePollStepReq
{
	tagSchemePollStepReq()
	{
		memset(m_tvWallId,0,sizeof(m_tvWallId));
		m_dwTvWallProjId = 0;
		m_byTvId = 0;
		m_byDivisionId = 0;
	}

	char 		m_tvWallId[MAX_DESC_LEN+1];	// 预案所属电视墙Id.
    u32 		m_dwTvWallProjId; //电视墙预案编号;
    u8 			m_byTvId; //电视机号;
    u8 			m_byDivisionId;//画面号(从零开始)
    TTPuChn 	m_tPuChn; //解码器通道;
}TSchPollStepReq;


typedef struct  tagTVWallSchemePollStep
{
	tagTVWallSchemePollStep()
	{
		memset(tvWallSchemeName,0,sizeof(tvWallSchemeName));
		duration = 0;
		m_byPollFlag = 0;
		m_wPollSrcIndex = 0;
		m_wPollSrcNum = 0;
		memset(m_dwTime,0,sizeof(m_dwTime));
	}
	char tvWallSchemeName[MAX_DESC_LEN+1];	// 预案名
	int duration;					// 预案切换前的保持时间.
	
    u8 m_byPollFlag; //轮询标志位;
    u16 m_wPollSrcIndex; //正在轮询的索引;
    u16 m_wPollSrcNum; //轮询源的编码器数目;
    u32 m_dwTime[MAXNUM_SRCPU_POLL]; 
    TTPuChn m_atEncPu[MAXNUM_SRCPU_POLL]; //轮询的源编码器组;
    TTPuChn m_tLookPu;//当前正在选看的PU,设备号为空表示选看无效
}TTvWallSchemePollStep;

enum emETVWallSCHEPOLLSTEPCMD
{
	emSDK_TVWSSCMD_INVALID = 0,
	emSDK_TVWSSCMD_CONFIG = 1,		// 配置
	emSDK_TVWSSCMD_START = 2,       // 开始*/
	emSDK_TVWSSCMD_STOP = 3,        // 停止*/ 
	emSDK_TVWSSCMD_PAUSE = 4,       // 暂停*/
	emSDK_TVWSSCMD_RESUME = 5,      // 恢复*/
	emSDK_TVWSSCMD_SEEK = 6,        // 时间定位*/
	emSDK_TVWSSCMD_FASTPLAY = 7,    // 快放 参数？*/		
	emSDK_TVWSSCMD_SLOWPLAY = 8,    // 慢放 参数？ */
	emSDK_TVWSSCMD_ONEFRAME = 9,    // 单帧播放*/
	emSDK_TVWSSCMD_KEYFRAME = 10,    // 只放关键帧*/
	emSDK_TVWSSCMD_REVERSEPLAY = 11,	/*倒放*/
	emSDK_TVWSSCMD_FORWARDPLAY = 12,	/*正放*/
	emSDK_TVWSSCMD_KEYSEEK = 13,		/*定位到关键帧. */
	emSDK_TVWSSCMD_UNKNOWN = 14,          
};
//SchemePollStep End

typedef struct tagCDomainInfo
{
	tagCDomainInfo()
	{
		memset(m_domainName,0,sizeof(m_domainName));
	}
	DomainID m_domainId;
	char m_domainName[MAX_DESC_LEN+1];
	DomainID m_parentDomainId;
	
}TDomainInfo;

typedef struct tagCNruInfo
{
	tagCNruInfo()
	{
		memset(m_nruId,0,sizeof(m_nruId));
		memset(m_devName,0,sizeof(m_devName));
		memset(m_devType,0,sizeof(m_devType));
		memset(m_manuFactory,0,sizeof(m_manuFactory));
		memset(m_ifName,0,sizeof(m_ifName));
		memset(m_ipAddr,0,sizeof(m_ipAddr));
		memset(m_netMask,0,sizeof(m_netMask));
		memset(m_gateway,0,sizeof(m_gateway));
		memset(m_RMSAddr,0,sizeof(m_RMSAddr));
		m_maxBandWidthKb = 0;
		m_online = FALSE;
		m_used = FALSE;
	}
	char m_nruId[MAX_DESC_LEN+1];         // nru id。
	char m_devName[MAX_DESC_LEN+1];       // 设备名
	char m_devType[MAX_NAME_LEN+1];       // 设备类型:VS200,KDM2801E等
	char m_manuFactory[MAX_DESC_LEN+1];   // 设备厂商

	char m_ifName[MAX_DESC_LEN+1];        // 网卡名.
	char m_ipAddr[ADDR_STR_LEN+1];        // IP地址.
	char m_netMask[ADDR_STR_LEN+1];       // 子网掩码.
	char m_gateway[ADDR_STR_LEN+1];       // 网关.

	char m_RMSAddr[ADDR_STR_LEN+1];       // 注册RMS服务器地址.
	int m_maxBandWidthKb;       // NRU的最大数据带宽, 单位 Kb/S.
	bool m_online;              // 是否在线.
	bool m_used;                // 是否启用.
}TCNruInfo;

typedef struct tagUserData
{
	tagUserData()
	{
		memset(dataKey,0,sizeof(dataKey));
		memset(dataValue,0,sizeof(dataValue));
		dataType = 0;
		dataFragNo = 0;
		dataAllFragNum = 0;
	}
	char dataKey[MAX_NAME_LEN+1];
	int  dataType;
	int  dataFragNo;
	int  dataAllFragNum;
	char dataValue[MAX_NAME_LEN+1];
}TUserData;

typedef struct tagStreamTransLocal
{
	tagStreamTransLocal()
	{
		STLocalVideoRtpPort = 0;
		STLocalVideoRtcpPort = 0;
		STLocalAudioRtpPort = 0;
		STLocalAudioRtcpPort = 0;
		memset(&STLocalIP,0,sizeof(STLocalIP));
	}
	
	u16 STLocalVideoRtpPort;
	u16 STLocalVideoRtcpPort;
	u16 STLocalAudioRtpPort;
	u16 STLocalAudioRtcpPort;
	char STLocalIP[ADDR_STR_LEN+1];
}TStreamTransLocal;

typedef struct tagStreamTransRemote
{
	tagStreamTransRemote()
	{
		STRemoteVideoRtpPort = 0;
		STRemoteVideoRtcpPort = 0;
		STRemoteAudioRtpPort = 0;
		STRemoteAudioRtcpPort = 0;
		STRemoteNatPort = 0;
		memset(STRemoteIP,0,sizeof(STRemoteIP));
		memset(STRemoteNatIP,0,sizeof(STRemoteNatIP));
		memset(RemoteStreamType,0,sizeof(RemoteStreamType));
	}
	u16 STRemoteVideoRtpPort;
	u16 STRemoteVideoRtcpPort;
	u16 STRemoteAudioRtpPort;
	u16 STRemoteAudioRtcpPort;
	char STRemoteIP[ADDR_STR_LEN+1];
	char STRemoteNatIP[ADDR_STR_LEN+1];
	char RemoteStreamType[MAX_ID_LEN+1];
	int STRemoteNatPort;

}TStreamTransRemote;

typedef struct tagWatchTask
{
	tagWatchTask()
	{
		m_byWatchTaskEnable = 0;
		m_byWatchTaskMinutes =0;
		for(int Idx=0;Idx< MAX_WATCH_ON_NUM;Idx ++)
		{
			m_byWatchTaskIntervals[Idx] = 0;
		}
	}

	u8	  m_byWatchTaskEnable;                     // ptz 操作后是否启动定时守望，非0则启用。
	u8	  m_byWatchTaskMinutes;                        // ptz 操作后几分钟启动定时守望
	u8	  m_byWatchTaskIntervals[MAX_WATCH_ON_NUM];    // 设置定时守望时间
	TPTZAUTHORITY m_PtzAuthority;
}TWatchTask;

typedef struct tagWatchPrPos
{
	tagWatchPrPos()
	{
		m_byPrePosEnable = 0;
		m_byPrePosTime =0;
		m_byPrePos =0;
	}
	u8    m_byPrePosEnable;                      // 停止ptz操作后载入预置位使能，非0则启用。
	u8    m_byPrePosTime;                        // 停止ptz操作后几分钟后载入预置位
	u8    m_byPrePos;                            // 载入几号预置
	TPTZAUTHORITY m_PtzAuthority;
}TWatchPrePos;

enum AudioEncodeType
{
	AUDIO_TYPE_PCMA = 0x00000001,
	AUDIO_TYPE_PCMU = 0x00000002,
	AUDIO_TYPE_GSM = 0x00000004,
	AUDIO_TYPE_G729 = 0x00000008,
	AUDIO_TYPE_ADPCM = 0x00000010,
	AUDIO_TYPE_G7221C = 0x00000020,
	AUDIO_TYPE_G722 = 0x00000040,
	AUDIO_TYPE_AACLC = 0x00000080,
};

typedef struct tagAudioParams
{
public:
	u32 m_dwSupportedAudioType;//支持的数据压缩格式（只读）;
	u32 m_dwAudioType;		//音频数据格式; 
	u8 m_byVolume;			//音量 0-25;
	u8 m_byAECState;		//回声抵消状态 DISABLE-无效，ENABLE-有效;
	u32 m_dwSmpRate;        //采样率 
	u8 m_bySmpBits;         //采样位数 
	u8 m_byChannle;         //声道 
	u32 m_dwSamplePerF;     //每帧样本数
	u32 m_dwReserved;		//保留字段

	//是否支持某种音频类型
	BOOL32 IsAudioTypeSupported(u32 dwAudioType) const
	{
		return m_dwSupportedAudioType & dwAudioType;
	}
	//设置音频类型支持
	void SetAudioTypeSupport(u32 dwAudioType)
	{
		m_dwSupportedAudioType = m_dwSupportedAudioType | dwAudioType;
	}
	//获取音频数据格式
	u32 GetAudioType() const
	{
		return m_dwAudioType;
	}
	//设置音频数据格式
	void SetAudioType(u32 dwAudioType)
	{
		m_dwAudioType = dwAudioType;
	}

	//获取音量 0-25
	u8 GetVolume() const
	{
		return m_byVolume;
	}
	//设置音量 0-25
	void SetVolume(u8 byVolume)
	{
		m_byVolume = byVolume;
	}

	//获取回声抵消状态 DISABLE-无效，ENABLE-有效
	BOOL32 IsAECState() const
	{
		return m_byAECState;
	}
	//设置回声抵消状态 DISABLE-无效，ENABLE-有效
	void SetIsAECState(BOOL32 bAECState)
	{
		m_byAECState = bAECState;
	}
	//获取采样率，单位为bps
	u32 GetSmpRate()
	{
		return m_dwSmpRate;
	}
	//设置采样率
	void SetSmpRate(u32 dwSmpRate)
	{
		m_dwSmpRate = dwSmpRate;
	}
	//获取采样位数，8位、16位、32位等
	u8 GetSmpBits()
	{
		return m_bySmpBits;
	}
	//设置采样位数
	void SetSmpBits(u8 bySmpBits)
	{
		m_bySmpBits = bySmpBits;
	}
	//获取单双声道，1-单声道，2-双声道
	u8 GetChannel()
	{
		return m_byChannle;
	}
	//设置单双声道
	void SetChannel(u8 byChannel)
	{
		m_byChannle = byChannel;
	}
	//获取每帧样本数
	u32 GetSamples()
	{
		return m_dwSamplePerF;
	}
	//设置每帧样本数
	void SetSamples(u32 dwSamples)
	{
		m_dwSamplePerF = dwSamples;
	}
}TAudioParams;

#pragma pack()
#endif
