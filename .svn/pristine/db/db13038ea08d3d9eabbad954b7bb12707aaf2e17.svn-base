/*****
*本文件定义了各种宏常量和宏函数，用于由loginid，playid
*downloadid等接口函数产生标示符生产hashid，并用hashid和CMap结合
*用hashid唯一地标示其他变量，如与loginid关联的通道号，与playid关联的播放进度等。
*version  09/12/2009  yuyonggui  Initial Version.
******/

#if !defined(CMAP_TEST_YUYONGGUI_20090911_001)
#define  CMAP_TEST_YUYONGGUI_20090911_001

//包含cmap头文件
//#include "afxtempl.h"

//定义LoginID与PlayID以及Downloadid的不同转换关系,不同的转换关系目的在于使得即使同样的
//id值，也能产生不同hashid值，并且差距能够拉开一段距离
#define MakeLoginIdHashId(LoginID) ((LoginID+5)*2)
#define MakePlayIdHashId(PlayID)   ((PlayID+11)*3)
#define MakeDownloadHashId(nDownloadID) ((nDownloadID+15))

//定义使用hashid标示的不同的变量的不同的附加值，以达到一一对应的目的
#define LoginIdAdd   1   //loginID 附加值 
#define ChanNumAdd   2   //通道号附加值
#define TotalTimeAdd 3   //文件总时间附加值
#define PosAdd       4   //进度附加值
#define TotalSizeAdd 5   //文件总大小附加值


//定义由playid标示其他变量的对应关系
#define GetHashOfLoginIdFormPlayId(PlayID)   (MakePlayIdHashId(PlayID)+LoginIdAdd)
#define GetHashOfChanNumFormPlayId(PlayID)   (MakePlayIdHashId(PlayID)+ChanNumAdd)
#define GetHashOfTotalTimeFormPlayId(PlayID) (MakePlayIdHashId(PlayID)+TotalTimeAdd)
#define GetHashOfPosFormPlayId(PlayID)       (MakePlayIdHashId(PlayID)+PosAdd)
#define GetHashOfTotalSizeFormPlayId(PlayID) (MakePlayIdHashId(PlayID)+TotalSizeAdd)

//定义由downloadid标示其他变量的对应关系
#define GetHashOfPosFormDownloadId(nDownloadID)  (MakeDownloadHashId(nDownloadID)+PosAdd)
#define GetHashOfTotalSizeFormDownloadId(nDownloadID)  (MakeDownloadHashId(nDownloadID)+TotalSizeAdd)


#endif