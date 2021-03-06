///*
//驱动名称;：
//pdf文件读取驱动:
//功能;：
//定向从pdf中读取数值;
//开发人员:xx;
//版本：1.0.0
//开始日期：2019-10-18
//结束日期：2019-10-18
//*/
//
//#include "pkdriver/pkdrvcmn.h"
//
///*
//*  删除字符串中空格，制表符tab等无效字符;
//*  @version  02/18/2017 xingxing  Initial Version;
//*/
//string Trim(string& str)
//{
//	//str.find_first_not_of(" \t\r\n"),在字符串str中从索引0开始，返回首次不匹配"\t\r\n"的位置
//	str.erase(0, str.find_first_not_of(" \t\r\n"));
//	str.erase(str.find_last_not_of(" \t\r\n") + 1);
//	return str;
//}
//
///*
//*  初始化函数;
//*  @version  02/18/2017 xingxing  Initial Version;
//*/
//PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
//{
//	return 0;
//}
//
//// 循环查找目录;
//int RecursiveDir(CExcelDevice *pExcelDevice, string strDirPath, map<time_t, string> &mapModifyTime2FilePath)
//{
//	return 0;
//}
//
//PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
//{
//	return PK_SUCCESS;
//}
//
//
////控制;
//PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
//{ 
//	return 0;
//}
////反初始化设备;
//PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
//{
//	return 0;
//}

/*
驱动名称;：
EXCEL文件读取驱动:
功能;：
定向从PDF中读取数值，写点;
开发人员:xx;
版本：1.0.0
开始日期：2016-02-18
结束日期：2016-02-18
*/

#include "pkdriver/pkdrvcmn.h"
#include "TableDevice.h"

/*
*  删除字符串中空格，制表符tab等无效字符;
*  @version  02/18/2017 xingxing  Initial Version;
*/
string Trim(string& str)
{
	//str.find_first_not_of(" \t\r\n"),在字符串str中从索引0开始，返回首次不匹配"\t\r\n"的位置;
	str.erase(0, str.find_first_not_of(" \t\r\n"));
	str.erase(str.find_last_not_of(" \t\r\n") + 1);
	return str;
}

/*
*  初始化函数;
*  @version  02/18/2017 xingxing  Initial Version;
*/
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	CExcelDevice *pExcelDevice = new CExcelDevice(pDevice);
	pDevice->pUserData[0] = pExcelDevice;
	pExcelDevice->m_strFilePath = pDevice->szConnParam;
	pExcelDevice->m_nSheetNo = ::atoi(pDevice->szParam1) - 1; // 1-based
	if (pExcelDevice->m_nSheetNo < 0)
		pExcelDevice->m_nSheetNo = 0;
	if (strcmp(pDevice->szParam2, "0") == 0)
		pExcelDevice->m_bRecursiveSubDir = false;
	else
		pExcelDevice->m_bRecursiveSubDir = true;
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device:%s, filepath:%s, sheetNo:%d, 是否需要递归子目录:%d", pDevice->szName, pExcelDevice->m_strFilePath.c_str(), pExcelDevice->m_nSheetNo + 1, pExcelDevice->m_bRecursiveSubDir);

	if (pExcelDevice->m_strFilePath.empty())
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, filepath:%s is EMPTY!!PLEASE CHECK! 文件路径不能为空!", pDevice->szName, pExcelDevice->m_strFilePath.c_str());
		return -1;
	}

	for (int iTag = 0; iTag < pDevice->nTagNum; iTag++)
	{
		PKTAG *pTag = pDevice->ppTags[iTag];
		CRowCalcMethod *pCalcMethod = new CRowCalcMethod(pDevice);
		pCalcMethod->m_pDevice = pDevice;
		pCalcMethod->m_pTag = pTag;
		pTag->pData1 = pCalcMethod;

		string strAddress = pTag->szAddress;								  // 1. Count;C=today(YYYY/MM/DD),I='OK'  2. Now(YYYY-MM-DD)  3. Cell(B13)
		vector<string> vecParam = PKStringHelper::StriSplit(strAddress, ";"); // 取出方法和后面的条件判断;
		if (vecParam.size() <= 0)											  // 可以没有参数;
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, file:%s, tag:%s, address:%s must include METHOD;PARAMS",
				pDevice->szName, pExcelDevice->m_strFilePath.c_str(), pTag->szName, pTag->szAddress);
			continue;
		}
		string strMethodSection = vecParam[0];
		string strWhereConds = "";
		if (vecParam.size() >= 2)				// 方法带有条件的情况;
			strWhereConds = vecParam[1];		// C=today(YYYY/MM/DD),I='OK';

		int nRet = pCalcMethod->ParseMethodSection(strMethodSection.c_str());
		if (nRet != 0 || pCalcMethod->m_nCalcMethod == COL_MATCH_FUNCTION_NONE)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, address:%s, method not supported!",
				pDevice->szName, pTag->szName, pTag->szAddress, strMethodSection.c_str());
			continue;
		}
		// 没有参数的函数;
		if (pCalcMethod->m_nCalcMethod == METHOD_GETNOW_TIMEFORMAT || pCalcMethod->m_nCalcMethod == METHOD_GETCELL_VALUE_XY)
		{
			continue;
		}
		vector<string> vecCondition = PKStringHelper::StriSplit(strWhereConds, ","); // C=Now(YYYY/M/D),I=NG Product
		for (int iCond = 0; iCond < vecCondition.size(); iCond++)
		{
			string strCondOne = vecCondition[iCond];
			CColStatMatch *pColMath = new CColStatMatch();
			pColMath->m_pDevice = pDevice;
			pColMath->m_pTag = pTag;
			nRet = pColMath->ParseMatchConditionInfo(strCondOne);
			if (nRet != 0) // 列不满足条件，配置错误;
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, file:%s, tag:%s, address:%s, METHOD must be:COLNAME=Func(COLNAME), or COLNAME=TEXT",
					pDevice->szName, pExcelDevice->m_strFilePath.c_str(), pTag->szName, pTag->szAddress);
				delete pColMath;
				continue;
			}
			pCalcMethod->m_vecColMatch.push_back(pColMath); // 增加一个匹配条件;
		}
	}
	PKTIMER tTimer;
	tTimer.nUserData[0] = 1;
	tTimer.nPeriodMS = 5000;
	Drv_CreateTimer(pDevice, &tTimer); // 设定定时器，系统设定;
	Drv_SetConnectOKTimeout(pDevice, 10);
	return 0;
}

// 循环查找目录;
int RecursiveDir(CExcelDevice *pExcelDevice, string strDirPath, map<time_t, string> &mapModifyTime2FilePath)
{
	bool bIsDir = PKFileHelper::IsDirectory(strDirPath.c_str());
	if (!bIsDir)
	{
		if (!PKFileHelper::IsFileExist(strDirPath.c_str()))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "配置的文件或目录:%s 不存在!", strDirPath.c_str());
			return 0;
		}

		time_t lTime = pExcelDevice->GetModifyTime(strDirPath);
		if (lTime < 0) // 无此文件;
			return -1;
		mapModifyTime2FilePath[lTime] = strDirPath;
		return 0;
	}

	// 下面是目录;
	//是目录，就遍历取文件;
	std::list<std::string> fileNameList;
	PKFileHelper::ListFilesInDir(strDirPath.c_str(), fileNameList);
	for (list<string>::iterator it_list = fileNameList.begin(); it_list != fileNameList.end(); it_list++)
	{
		string strFilePath = strDirPath + PK_OS_DIR_SEPARATOR + *it_list;
		time_t lTime = pExcelDevice->GetModifyTime(strFilePath);
		if (lTime < 0) // 无此文件;
			continue;
		mapModifyTime2FilePath[lTime] = strFilePath;
	}

	// 遍历子目录;
	if (pExcelDevice->m_bRecursiveSubDir)
	{
		std::list<std::string> dirNameList;
		PKFileHelper::ListSubDir(strDirPath.c_str(), dirNameList);
		for (list<string>::iterator it_list = dirNameList.begin(); it_list != dirNameList.end(); it_list++)
		{
			string strSubDir = strDirPath + PK_OS_DIR_SEPARATOR + *it_list;
			RecursiveDir(pExcelDevice, strSubDir, mapModifyTime2FilePath);
		}
	}
	return 0;
}

PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CExcelDevice *pExcelDevice = (CExcelDevice *)pDevice->pUserData[0];
	//读取设备信息写入设备类中，文件地址、sheet页名称、点名;
	vector<string> vecDevInfo = PKStringHelper::StriSplit(pExcelDevice->m_strFilePath, ";");

	map<time_t, string> mapFileTime2Path;
	for (auto it_vec : vecDevInfo) // 可以是多个文件名;
	{
		string strFileOrDirPath = it_vec;
		RecursiveDir(pExcelDevice, strFileOrDirPath, mapFileTime2Path);
	}

	if (mapFileTime2Path.size() <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "FileOrDir:%s 未配置任何路径为文件, 无法进行查找", pExcelDevice->m_strFilePath.c_str());
		return -1;
	}

	map<time_t, string>::reverse_iterator itMaxTime = mapFileTime2Path.rbegin();
	time_t tmMax = itMaxTime->first;
	string strFilePath = itMaxTime->second;
	mapFileTime2Path.clear();

	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, 找到的最大修改日期的文件:%s, 修改时间:%d", pDevice->szName, strFilePath.c_str(), tmMax);

	//获取文件后缀，主要是判断是否是.csv文件后缀;
	vector<string> vecFileInfo = PKStringHelper::StriSplit(strFilePath.c_str(), ".");
	if (vecFileInfo.size() <= 1)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, File:%s, Must include ext File(扩展名)", pDevice->szName, strFilePath.c_str());
		return -1;
	}

	// 检查文件是否存在;
	if (!PKFileHelper::IsFileExist(strFilePath.c_str()))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, File:%s, 文件不存在!", pDevice->szName, strFilePath.c_str());
		return -102;
	}

	// 检查文件格式是否合法，并区分出来excel或者csv;
	bool bExcelFile = false;
	string strFileExt = vecFileInfo[vecFileInfo.size() - 1]; // 取最后一段;
	if (PKStringHelper::StriCmp(strFileExt.c_str(), "csv") == 0) // csv;
	{
		bExcelFile = false;
	}
	else if (PKStringHelper::StriCmp(strFileExt.c_str(), "xls") == 0 || PKStringHelper::StriCmp(strFileExt.c_str(), "xlsx") == 0 ||
		PKStringHelper::StriCmp(strFileExt.c_str(), "xlsm") == 0)
	{
		bExcelFile = true;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, File:%s, 扩展名必须是:xls,xlsx,xlsm,csv", pDevice->szName, strFilePath.c_str());
		return -101;
	}

	//sheet页数;
	int nSheetNum = pExcelDevice->m_nSheetNo;

	Drv_SetConnectOK(pDevice);
	if (bExcelFile)
	{
		pExcelDevice->ProcessExcelFile(strFilePath.c_str());
	}
	else
		pExcelDevice->ProcessCSVFile(strFilePath.c_str());

	return PK_SUCCESS;
}


//控制;
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{
	return 0;
}
//反初始化设备;
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	return 0;
}
