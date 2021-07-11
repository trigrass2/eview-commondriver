#include "TableDevice.h"

time_t StringToTimeWithFormat(const char *szTimeString, const char *szTimeFormat);

void clearRows(vector<vector<string> >&rows)
{
	for (int iRow = 0; iRow < rows.size(); iRow++)
		rows[iRow].clear();
	rows.clear();
}

// 将函数拆分为2部分。Max()-->Max,  Now(YYYY/M/D)-->Now, YYYY/M/D;MaxTime(hh:mm:ss)-->MaxTime, hh:mm:ss
// 返回0表示是函数，返回其他表示不是函数
int SplitFuncNameWithParam(PKDEVICE *pDevice, PKTAG *pTag, string strFuncWithParam, string &strFuncName, string &strParam)
{
	int nPosLeft = strFuncWithParam.find("(");
	int nPosRight = strFuncWithParam.find(")");
	if (nPosLeft < 0 || nPosRight < 0 || nPosRight < nPosLeft)
	{
		// Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, tag:%s, param:%s is not FUNCTION, not comford to FUNCNAME(FUNCPARAM)", pDevice->szName, pTag->szName, strFuncWithParam.c_str());
		strFuncName = strFuncWithParam; // 名称
		return -1;
	}

	strFuncName = strFuncWithParam.substr(0, nPosLeft);
	strParam = strFuncWithParam.substr(nPosLeft + 1, nPosRight - nPosLeft - 1); // YYYY-MM-DD
	return 0;
}

// 将单元格名字拆分为列和行，返回序号;
// 返回的数字为行号索引，如AA23返回2.如果返回-1表示都是列
int SplitRowCol(string &strCellName) 
{
	transform(strCellName.begin(), strCellName.end(), strCellName.begin(), ::toupper);

	if (strCellName.length() <= 0)
		return -1;

	int nRowIndex = 0;
	for (int i = 0; i < strCellName.length(); i++) // AA23-->
	{
		char chColName = strCellName[i];
		if (chColName < 'A' || chColName > 'Z') // 是数字，说明是行号。AB354，遇到3时不满足
		{
			break;
		}
		nRowIndex++;
	}
	return nRowIndex;
}

int GetColIndex(string strColName, int &nColIndex)
{
	if (strColName.length() <= 0)
	{
		nColIndex = -1;
		return -1;
	}

	transform(strColName.begin(), strColName.end(), strColName.begin(), ::toupper);
	nColIndex = 0;
	for (int i = 0; i < strColName.length(); i++) // AA-->;
	{
		char chColName = strColName[i];
		int nBitVal = chColName - 'A' + 1;
		nColIndex = nColIndex * 26 + nBitVal;
	}
	nColIndex -= 1; // 变成1为基数;
	return 0;
	// 返回列号索引;
}

int GetRowColIndex(string strCellName, int &nRowIndex, int &nColIndex) // B15--->1,14;
{
	int nRowBegin = SplitRowCol(strCellName);
	if (nRowBegin < 0) // 为空;
		return -1;

	string strColName = strCellName.substr(0, nRowBegin); // AB123--->ab;
	string strRowName = strCellName.substr(nRowBegin); // AB123-->123;
	transform(strCellName.begin(), strCellName.end(), strCellName.begin(), ::toupper);

	GetColIndex(strColName, nColIndex);
	nRowIndex = ::atoi(strRowName.c_str()) - 1;
	if (nRowIndex < 0)
		nRowIndex = 0;
	if (nColIndex < 0)
		nColIndex = 0;
	return 0;
}

void CColStatMatch::ResetMatchTemp()
{
	m_strCurMaxText = "";
	m_dbCurMaxReal = MIN_RELA;
	m_nCurMaxInt = MIN_INTEGER;
	m_tmCurMaxTime = 0;
}

CColStatMatch::CColStatMatch()
{
	m_strColName = m_strWhereConditions = "";
	m_nColIndex = 0;
	m_nMatchMode = COL_MATCHMODE_EQUAL; 
	m_pDevice = NULL;
	m_pTag = NULL;
	ResetMatchTemp();
}

// {条件表达式}:D=MaxText(),D=MaxTime(D,hh;mm;ss),D=MaxInt(),D=MaxReal(),D=Now(YYYY/MM/DD)
int CColStatMatch::ParseMatchConditionInfo(string strColNameAndWhere)
{
	vector<string> vecCondOne = PKStringHelper::StriSplit(strColNameAndWhere, "=");
	if (vecCondOne.size() <= 1)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,tag:%s, address:%s must Count;ColName1=today(YYYY/MM/DD),ColName2=OK. 缺少=号!",
			m_pDevice->szName, m_pTag->szName, m_pTag->szAddress);
		return -100;
	}

	string strColName = vecCondOne[0];
	string strColWhere = vecCondOne[1];
	GetColIndex(strColName, m_nColIndex);

	this->m_nMatchMode = COL_MATCHMODE_EQUAL;
	this->m_strColName = strColName;
	this->m_strWhereConditions = strColWhere;

	// 解析列字段条件 MaxText(),  Now(YYYY/M/D)
	string strFuncName;
	string strFuncParam;
	int nRet = SplitFuncNameWithParam(m_pDevice, m_pTag, strColWhere, strFuncName, strFuncParam);
	if (nRet != 0) // 字符串, NG Product, OK, ERROR
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_NONE;
		m_strTextToCompare = strColWhere;
		return 0;
	}

	// IS FUNCITON  // MaxText(D),  MaxInt(D), MaxReal(D), Now(YYYY/M/D)
	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Now") == 0)
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_NOW;
		m_strTimeFormat = strFuncParam; // YYYY/M/D
	}
	else if (PKStringHelper::StriCmp(strFuncName.c_str(), "MaxTime") == 0)
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_MAXTIME;
		m_strTimeFormat = strFuncParam; // YYYY/M/D
	}
	else if (PKStringHelper::StriCmp(strFuncName.c_str(), "MaxText") == 0) // MaxText(D)
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_MAXTEXT;
	}
	else if (PKStringHelper::StriCmp(strFuncName.c_str(), "MaxInt") == 0) // MaxInt(D)
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_MAXINT; 
	}
	else if (PKStringHelper::StriCmp(strFuncName.c_str(), "MaxReal") == 0) // MaxReal(D)
	{
		m_nMatchFuncNo = COL_MATCH_FUNCTION_MAXREAL; 
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, address:%s, colmatch:%s UNSUPPORT FUNCTIONNAME:%s",
			m_pDevice->szName, m_pTag->szName, m_pTag->szAddress, m_strWhereConditions.c_str(), strFuncName.c_str());
		return -1;
	} 

	return 0;
}

// Max(D),  Now(YYYY/M/D), OK
bool CColStatMatch::Matched(const char *szColValue) // 是否匹配成功
{
	if (COL_MATCHMODE_EQUAL != m_nMatchMode)
		return false;

	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_NONE)
	{
		if (m_strTextToCompare.compare(szColValue) == 0) // 当前你列值==预定义的值相等
			return true;
		return false;
	}

	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_NOW)
	{
		string strTimeWithFormat;
		GetNowTimeString(strTimeWithFormat, m_strTimeFormat.c_str()); // 时间匹配成功了;
		if (strstr(szColValue, strTimeWithFormat.c_str()) != NULL) //当前单元格的时间值，包含了格式化后的今天日期;
			return true;

		return false;
	}
	
	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_MAXTIME)
	{
		string strTimeWithFormat;
		time_t tmColValue = StringToTimeWithFormat(szColValue, m_strTimeFormat.c_str());
		if (m_tmCurMaxTime < tmColValue)
		{
			m_tmCurMaxTime = tmColValue;
			return true;
		}
		return false;
	}

	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_MAXTEXT) // 这个必须是最后一个匹配！前面都匹配成功了，这个匹配也成功了！MaxText()
	{
		if (m_strCurMaxText < szColValue)
		{
			m_strCurMaxText = szColValue;
			return true;
		}
		return false;
	}

	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_MAXINT) // 这个必须是最后一个匹配！前面都匹配成功了，这个匹配也成功了！MaxInt()
	{
		int nColValue = ::atoi(szColValue);
		if (m_nCurMaxInt < nColValue)
		{
			m_nCurMaxInt = nColValue;
			return true;
		}
		return false;
	}

	if (m_nMatchFuncNo == COL_MATCH_FUNCTION_MAXREAL) // 这个必须是最后一个匹配！前面都匹配成功了，这个匹配也成功了！ MaxReal()
	{
		double dbColValue = ::atof(szColValue);
		if (m_dbCurMaxReal < dbColValue)
		{
			m_dbCurMaxReal = dbColValue;
			return true;
		}
		return false;
	}

	return true;
}

CRowCalcMethod::CRowCalcMethod(PKDEVICE *pDevice)
{
	m_pDevice = pDevice;
	m_nCalcMethod = METHOD_GETCELL_VALUE_XY;
	m_strNow_TimeFormat = "";
	m_nFuncParam_RowNo = -1;
	m_nFuncParam_ColNo = -1;
	m_vecColMatch.clear();
}

// Count,Now(YYYY-MM-DD),MaxInt(B)/MaxReal/MaxText,Cell(B13),SUM(A)
int CRowCalcMethod::ParseMethodSection(const char *szMethodSection)// 分析方法字段
{
	CRowCalcMethod *pCalcMethod = this;
	PKDEVICE *pDevice = m_pDevice;
	string strMethodSection = szMethodSection;
	string strFuncName;
	string strFuncParam;
	int nRet = SplitFuncNameWithParam(m_pDevice, m_pTag, strMethodSection, strFuncName, strFuncParam);
	if (nRet != 0) // 字符串
	{
	}

	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Cell") == 0) // CELL(B13)
	{
		pCalcMethod->m_nCalcMethod = METHOD_GETCELL_VALUE_XY;
		if (strFuncParam.length() < 2) // B7 at least
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, address:%s must Cell;B5 at least 2 chars!", m_pDevice->szName, m_pTag->szName, m_pTag->szAddress);
			pCalcMethod->m_nCalcMethod = COL_MATCH_FUNCTION_NONE;
			return -100;
		}

		GetRowColIndex(strFuncParam, pCalcMethod->m_nFuncParam_RowNo, pCalcMethod->m_nFuncParam_ColNo);// B13, 13是第13行，从1开始，变为从0开始
		return 0;
	}

	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Count") == 0) // C=now(YYYY/MM/DD),I='OK'
	{
		pCalcMethod->m_nCalcMethod = METHOD_COUNT_MATCHCOL;
		return 0;
	}

	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Now") == 0)
	{
		pCalcMethod->m_nCalcMethod = METHOD_GETNOW_TIMEFORMAT;
		m_strNow_TimeFormat = strFuncParam;
		return 0;
	}

	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Sum") == 0) // SUM(A)
	{
		pCalcMethod->m_nCalcMethod = METHOD_SUM_MATCHCOL;
		GetColIndex(strFuncParam, pCalcMethod->m_nFuncParam_ColNo);
		return 0;
	}

	if (PKStringHelper::StriCmp(strFuncName.c_str(), "Col") == 0) // Col(A)
	{
		pCalcMethod->m_nCalcMethod = METHOD_COLUMNVALUE;
		GetColIndex(strFuncParam, pCalcMethod->m_nFuncParam_ColNo);
		return 0;
	}

	Drv_LogMessage(PK_LOGLEVEL_ERROR, "！！device:%s, tag:%s, address:%s, Func:%s DO NOT SUPPORT!",
		m_pDevice->szName, m_pTag->szName, m_pTag->szAddress, strMethodSection.c_str());
	pCalcMethod->m_nCalcMethod = METHOD_UNKNOWN;
	return -101;
}

CExcelDevice::CExcelDevice(PKDEVICE *pDevice)
{
	m_pDevice = pDevice;
	m_strFilePath = m_strSheetNo = "";
	m_nSheetNo = 0;
	m_bRecursiveSubDir = true;
}


//
/*
*  初始化excel函数;
*  @version  02/18/2017 xingxing  Initial Version;
*/
Book* CExcelDevice::InitAndCreateBook(string strPath)
{
	vector<string> vecFileInfo = PKStringHelper::StriSplit(strPath.c_str(), ".");
	Book *book = NULL;
	if (vecFileInfo[1] == "xlsm" || vecFileInfo[1] == "xlsx")
	{
		book = xlCreateXMLBook();
		if (!book)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "create book failed xlsm,xlsx");
			return NULL;
		}
	}
	else
	{
		book = xlCreateBook();
		if (!book)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "create book failed xls");
			return NULL;
		}
	}
	//设置KEY值
	book->setKey(KEY, VAL);
	//装载已有文件
	if (!book->load(strPath.c_str()))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "load excel File failed :%s", strPath.c_str());
		return NULL;
	}
	return book;
}


/*
*  取修改时间与当前时间间隔;
*  @version  02/18/2017 xingxing  Initial Version;
*/
time_t CExcelDevice::GetModifyTime(string strPath)
{
	struct stat stBuf;
	char szModifyTime[26] = { 0 };
	const char *szFileName = strPath.c_str();
	int nRet = stat(szFileName, &stBuf);
	long nTime = 0;
	if (0 != nRet) // ERROR
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "获取文件修改时间失败:%s", strPath.c_str());
		int nErrorNo = errno;
		switch (nErrorNo)
		{
		case ENOENT:
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "File %s, not found, errno=%d", szFileName, nErrorNo);
			return 0;
		case EINVAL:
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "File:%s, Invalid parameter(errno=%d=EINVAL) to _stat.", szFileName, nErrorNo);
			return 0;
		default:
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "File:%s, Invalid parameter(errno=%d) to _stat.", szFileName, nErrorNo);
			return 0;
		}
	}

	return stBuf.st_mtime;
}

/*
*  根据时间模式获取年月日
*  @version  02/18/2017 xingxing  Initial Version;
*/
int GetNowTimeString(string &strTime, const char *szTimeFormat)
{
	time_t t = time(NULL); 
	struct tm * tmStart = localtime(&t);
	int nYear = 1900 + tmStart->tm_year;
	int nMonth = 1 + tmStart->tm_mon;
	int nDay = tmStart->tm_mday;
	int nHour = tmStart->tm_hour;
	int nMinute = tmStart->tm_min;
	int nSecond = tmStart->tm_sec;

	char szTimeString[32] = { 0 };
	strTime = szTimeFormat;
	int nPos = string::npos;
	// DD,D
	if ((nPos = strTime.find("DD")) != string::npos)
	{
		sprintf(szTimeString, "%02d", nDay);
		strTime = strTime.replace(nPos, 2, szTimeString);
	}
	else if ((nPos = strTime.find("D")) != string::npos)
	{
		sprintf(szTimeString, "%d", nDay);
		strTime = strTime.replace(nPos, 1, szTimeString);
	}

	//MM, M
	if ((nPos = strTime.find("MM")) != string::npos)
	{
		sprintf(szTimeString, "%02d", nMonth);
		strTime = strTime.replace(nPos, 2, szTimeString);
	}
	else if ((nPos = strTime.find("M")) != string::npos)
	{
		sprintf(szTimeString, "%d", nMonth);
		strTime = strTime.replace(nPos, 1, szTimeString);
	}

	// YYYY, YY
	if ((nPos = strTime.find("YYYY")) != string::npos)
	{
		sprintf(szTimeString, "%04d", nYear);
		strTime = strTime.replace(nPos, 4, szTimeString);
	}
	else if ((nPos = strTime.find("YY")) != string::npos)
	{
		sprintf(szTimeString, "%04d", nYear);
		char *szTimeTmp = szTimeString + 2; // 2017-->17
		strTime = strTime.replace(nPos, 2, szTimeTmp);
	}

	// h, hh
	if ((nPos = strTime.find("hh")) != string::npos)
	{
		sprintf(szTimeString, "%02d", nHour);
		strTime = strTime.replace(nPos, 2, szTimeString);
	}
	else if ((nPos = strTime.find("h")) != string::npos)
	{
		sprintf(szTimeString, "%d", nHour);
		strTime = strTime.replace(nPos, 1, szTimeString);
	}

	// m, mm
	if ((nPos = strTime.find("mm")) != string::npos)
	{
		sprintf(szTimeString, "%02d", nMinute);
		strTime = strTime.replace(nPos, 2, szTimeString);
	}
	else if ((nPos = strTime.find("m")) != string::npos)
	{
		sprintf(szTimeString, "%d", nMinute);
		strTime = strTime.replace(nPos, 1, szTimeString);
	}

	// s, ss
	if ((nPos = strTime.find("ss")) != string::npos)
	{
		sprintf(szTimeString, "%02d", nSecond);
		strTime = strTime.replace(nPos, 2, szTimeString);
	}
	else if ((nPos = strTime.find("s")) != string::npos)
	{
		sprintf(szTimeString, "%d", nSecond);
		strTime = strTime.replace(nPos, 1, szTimeString);
	}

	return 0;
}


// strTimeStr:18 12:33:34, strTimeFormat:DD hh:mm:ss; strTimeType:DD
int ParseIntFromStringBySeparator(string &strTimeString, string &strTimeFormat, string strTimeType, int &nValue)
{
	int nPos = string::npos;
	if ((nPos = strTimeFormat.find(strTimeType)) != 0) // 不是以DD开头
		return -1;

	string strValue = "";
	if (strTimeFormat.length() <= strTimeType.length()) // 最后1个了
	{
		strValue = strTimeString;
		strTimeString = "";
		strTimeFormat = "";
	}
	else
	{
		char chSeparator = strTimeFormat.at(nPos + strTimeType.length());	// 得到DD和hh之间的分隔符，如空格
		strTimeFormat = strTimeFormat.substr(nPos + strTimeType.length() + 1); // DD hh;mm;ss--> "hh:mm:ss"
		int nPosLeft = strTimeString.find(chSeparator);
		if (nPosLeft == string::npos) // 没有多余的时间字符串要解析了
			return -1;

		strValue = strTimeString.substr(0, nPosLeft);
		strTimeString = strTimeString.substr(nPosLeft + 1); // 跳过后面的分隔符 
	}
	nValue = ::atoi(strValue.c_str());

	return 0;
}

// 12;24;25, hh:mm:ss--->转换为time_t
// 支持:YYYY,MM,DD,hh,mm,ss,YY,M,D,h,m,s
time_t StringToTimeWithFormat(const char *szTimeString, const char *szTimeFormat)
{
	int nYear = 1971;
	int nMonth = 0;
	int nDay = 1;
	int nHour = 1;
	int nMinute = 1;
	int nSecond = 1;

	string strTimeString = szTimeString;
	string strTimeFormat = szTimeFormat;
	while (strTimeFormat.length() > 0)
	{
		// 查找开始是不是我们设定的字符
		int nTimePart = 0; // 取时间的整数部分。不支持AM这种格式的时间
		int nPos = string::npos;
		int nRet = 0;
		if ((nPos = strTimeFormat.find("YYYY")) != string::npos) // YYYY
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "YYYY", nYear);
		}
		else if ((nPos = strTimeFormat.find("YY")) != string::npos) // YY
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "YY", nYear);
		}
		else if ((nPos = strTimeFormat.find("MM")) != string::npos) // M
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "M", nMonth);
		}
		else if ((nPos = strTimeFormat.find("M")) != string::npos) // M
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "M", nMonth);
		}
		else if ((nPos = strTimeFormat.find("DD")) != string::npos) // DD
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "DD", nDay);
		}
		else if ((nPos = strTimeFormat.find("D")) != string::npos) // D
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "D", nDay);
		}
		else if ((nPos = strTimeFormat.find("hh")) != string::npos) // hh
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "hh", nHour);
		}
		else if ((nPos = strTimeFormat.find("h")) != string::npos) // h
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "h", nHour);
		}
		else if ((nPos = strTimeFormat.find("mm")) != string::npos) // mm
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "mm", nMinute);
		}
		else if ((nPos = strTimeFormat.find("m")) != string::npos) // m
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "m", nMinute);
		}
		else if ((nPos = strTimeFormat.find("ss")) != string::npos) // ss
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "ss", nSecond);
		}
		else if ((nPos = strTimeFormat.find("s")) != string::npos) // s
		{
			nRet = ParseIntFromStringBySeparator(strTimeString, strTimeFormat, "s", nSecond);
		}
		else
		{
			strTimeString = strTimeString.substr(1); // 向前跳过1个字符
			strTimeFormat = strTimeFormat.substr(1); // 向前跳过1个字符
		}

		if (strTimeFormat.length() <= 0 || strTimeString.length() <= 0) // 解析完毕了
			break;
	}

	if (nYear > 0)
		nYear -= 1900; // 转为0为基准
	if (nYear < 71)
		nYear = 71;

	if (nMonth > 0)
		nMonth -= 1; // 转为0为基准
	tm tmTime;
	memset(&tmTime, 0, sizeof(struct tm));
	tmTime.tm_year = nYear; // 以1900为基准. 至少是71(1970年），也不能太大，否则后面发怒会-1
	tmTime.tm_mon = nMonth;
	tmTime.tm_mday = nDay;
	tmTime.tm_hour = nHour;
	tmTime.tm_min = nMinute;
	tmTime.tm_sec = nSecond;
	tmTime.tm_isdst = 0;
	time_t tmVal = 0;
	tmVal = mktime(&tmTime);

	//time_t t = time(NULL);
	////t += 86400; // 下一天的同一时刻 
	//struct tm * tmStart = localtime(&t);
	//tmStart->tm_year = 69;
	//time_t t2 = mktime(tmStart);
	if (tmVal < 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "时间格式转换非法, Time:%s,Format: %s, timeValue:%d < 0", szTimeString, szTimeFormat, tmVal);
	}
	return tmVal;
}

// 处理csv格式，仅仅支持读值判断并累加
int CExcelDevice::ProcessExcelFile(const char *szFilePath)
{
	vector<vector<string> > vecRows;
	ReadRowsExcelFile(szFilePath, vecRows);
	CalcTagsFromRows(vecRows);
	clearRows(vecRows);
	return 0;
}

int CExcelDevice::ReadRowsCSVFile(const char *szFilePath, vector<vector<string> >& vecRows) // excel的N行N列
{
	clearRows(vecRows);

	//遍历文件
	ifstream fin(szFilePath);
	if (!fin.is_open())
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, ProcessCSV, file:%s open failed!", m_pDevice->szName, szFilePath);
		return -200;
	}

	// 读入到M行N列的数组。暂时不考虑单引号和多引号的转义
	string strLineData;
	while (getline(fin, strLineData))	//整行读取，换行符“\n”区分，遇到文件尾标志eof终止读取
	{
		istringstream sin(strLineData);
		vector<string> vecCol;
		string strField;
		while (getline(sin, strField, ',')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			vecCol.push_back(strField);//将刚刚读取的字符串添加到向量fields中
		}
		vecRows.push_back(vecCol);
	}
	fin.close();
	return 0;
}

int CExcelDevice::ReadRowsExcelFile(const char *szFilePath, vector<vector<string> >& vecRows) // excel的N行N列
{
	clearRows(vecRows);

	CExcelDevice *pExcelDevice = this;
	//遍历excel文件读取固定列的值，根据时间判断进行累计
	//初始化excel库函数
	Book *book = InitAndCreateBook(szFilePath);
	if (NULL == book)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, init excel file:%s failed!", pExcelDevice->m_pDevice->szName, szFilePath);
		return -100;
	}
	//遍历文件
	Sheet *pSheet = book->getSheet(pExcelDevice->m_nSheetNo); // 从第0个tab开始
	if (!pSheet)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, init excel file:%s, sheetNo:%d(zero-based) cannot open!",
			pExcelDevice->m_pDevice->szName, szFilePath, pExcelDevice->m_nSheetNo);
		return -101;
	}

	int nFirstRow = pSheet->firstRow(); // 肯定从0开始吧？
	int nLastRow = pSheet->lastRow();
	int nFirstCol = pSheet->firstCol();  // 肯定从0开始吧？
	int nLastCol = pSheet->lastCol();
	for (int iRow = nFirstRow; iRow < nLastRow; iRow++)
	{
		vector<string> vecCol;
		for (int iCol = nFirstCol; iCol < nLastCol; iCol++)
		{
			string strCellValue = pSheet->readStr(iRow, iCol);
			vecCol.push_back(strCellValue);
		}
		vecRows.push_back(vecCol);
	}

	book->release();
	return 0;
}

// 处理csv格式，仅仅支持读值判断并累加
int CExcelDevice::ProcessCSVFile(const char *szFilePath)
{
	vector<vector<string> > vecRows;
	ReadRowsCSVFile(szFilePath, vecRows);
	CalcTagsFromRows(vecRows);
	clearRows(vecRows);
	return 0;
}

int CExcelDevice::CalcTagsFromRows(vector<vector<string> > &vecRows)
{
	//遍历点表
	vector<PKTAG *> vecTagToUpdate;
	for (int iTag = 0; iTag < m_pDevice->nTagNum; iTag++)
	{
		PKTAG *pTag = m_pDevice->ppTags[iTag];
		CRowCalcMethod *pCalcMethod = (CRowCalcMethod *)pTag->pData1;
		if (pCalcMethod == NULL)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, file:%s, sheetNo:%d, address:%s INVALID!!!!",
				m_pDevice->szName, pTag->szName, this->m_strFilePath.c_str(), m_nSheetNo + 1, pTag->szAddress);
			continue;
		}

		if (pCalcMethod->m_nCalcMethod == METHOD_GETCELL_VALUE_XY)
		{
			if (pCalcMethod->m_nFuncParam_RowNo >= vecRows.size())
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, file:%s, sheetNo:%d, hopeRow:%d > file max RowNo:%d",
					m_pDevice->szName, pTag->szName, this->m_strFilePath.c_str(), m_nSheetNo + 1, pCalcMethod->m_nFuncParam_RowNo, vecRows.size());
				continue;
			}

			vector<string> &vecCol = vecRows[pCalcMethod->m_nFuncParam_RowNo];
			if (pCalcMethod->m_nFuncParam_ColNo >= vecCol.size())
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, file:%s, sheetNo:%d, hopeRow:%d, hopeCol::%d > maxColNo of this Row:%d",
					m_pDevice->szName, pTag->szName, this->m_strFilePath.c_str(), m_nSheetNo + 1, pCalcMethod->m_nFuncParam_RowNo, pCalcMethod->m_nFuncParam_ColNo, vecCol.size());
				continue;
			}

			string strCellValue = vecCol[pCalcMethod->m_nFuncParam_ColNo];
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, tag:%s, value:%s, address:%s, file:%s, sheetNo:%d",
				m_pDevice->szName, pTag->szName, strCellValue.c_str(), pTag->szAddress, this->m_strFilePath.c_str(), m_nSheetNo + 1);
			Drv_SetTagData_Text(pTag, strCellValue.c_str());
			vecTagToUpdate.push_back(pTag);
			continue;
		}
		
		if (pCalcMethod->m_nCalcMethod == METHOD_GETNOW_TIMEFORMAT)
		{
			string strNowWithFormat;
			GetNowTimeString(strNowWithFormat, pCalcMethod->m_strNow_TimeFormat.c_str());
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, tag:%s, value:%s, address:%s, file:%s, sheetNo:%d",
				m_pDevice->szName, pTag->szName, strNowWithFormat.c_str(), pTag->szAddress, this->m_strFilePath.c_str(), m_nSheetNo + 1);
			Drv_SetTagData_Text(pTag, strNowWithFormat.c_str());
			vecTagToUpdate.push_back(pTag);
			continue;
		}
		
		for (int j = 0; j < pCalcMethod->m_vecColMatch.size(); j++)
		{
			CColStatMatch *pColMatch = pCalcMethod->m_vecColMatch[j];
			pColMatch->ResetMatchTemp(); // 重置临时变量
		}

		// Col(A), Sum(A), Count(A)
		int nCountValue = 0;
		string strDestColValue = ""; // 目标列值
		double dbSumValue = 0;
		int nMatchRow = 0;
		for (int iRow = 0; iRow < vecRows.size(); iRow++) // 每一行
		{
			vector<string> & vecCol = vecRows[iRow];
			bool bMatched = true;
			int nMatchCount = 0;
			for (int j = 0; j < pCalcMethod->m_vecColMatch.size(); j++)
			{
				CColStatMatch *pColMatch = pCalcMethod->m_vecColMatch[j];
				if (pColMatch->m_nColIndex >= vecCol.size())
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, file:%s, sheetNo:%d, 第%d行第:%d列 > 该行的最大列号:%d",
						m_pDevice->szName, pTag->szName, this->m_strFilePath.c_str(), m_nSheetNo + 1, iRow + 1, pColMatch->m_nColIndex + 1, vecCol.size());
					bMatched = false;
					break;
				}
				string strData = vecCol[pColMatch->m_nColIndex];
				strData = Trim(strData);
				if (!pColMatch->Matched(strData.c_str()))
				{
					bMatched = false;
					break;
				}
			}

			if (bMatched) // 该行各个条件都匹配了, 认为满足！
			{
				nMatchRow = iRow;
				if (pCalcMethod->m_nCalcMethod == METHOD_COUNT_MATCHCOL)
				{
					nCountValue++;
					continue;
				}

				// 下面都需要取出列值！
				if (pCalcMethod->m_nFuncParam_ColNo < 0 || pCalcMethod->m_nFuncParam_ColNo >= vecCol.size()) // 不匹配
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, address:%s, file:%s, sheetNo:%d, row:%d, DestCol:%d < 0, OR DestColNo >= colsize:%d",
						m_pDevice->szName, pTag->szName, pTag->szAddress, this->m_strFilePath.c_str(), m_nSheetNo + 1, nMatchRow + 1, pCalcMethod->m_nFuncParam_ColNo, vecCol.size());
					continue;
				}

				strDestColValue = vecCol[pCalcMethod->m_nFuncParam_ColNo];

				if (pCalcMethod->m_nCalcMethod == METHOD_COLUMNVALUE) // 列值
				{
				}
				else if (pCalcMethod->m_nCalcMethod == METHOD_SUM_MATCHCOL)
				{
					double dbValue = ::atoi(strDestColValue.c_str());
					dbSumValue += dbValue;
				}
			}
		}

		char szTagValue[256] = { 0 };
		if (pCalcMethod->m_nCalcMethod == METHOD_COLUMNVALUE) // 列值
		{
			PKStringHelper::Safe_StrNCpy(szTagValue, strDestColValue.c_str(), sizeof(szTagValue));
		}
		else if (pCalcMethod->m_nCalcMethod == METHOD_SUM_MATCHCOL)
		{
			sprintf(szTagValue, "%f", dbSumValue);
		}
		else if (pCalcMethod->m_nCalcMethod == METHOD_COUNT_MATCHCOL)
		{
			sprintf(szTagValue, "%d", nCountValue);
		}

		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, tag:%s, value:%s, address:%s, file:%s, sheetNo:%d, rowNo:%d of %d",
			m_pDevice->szName, pTag->szName, szTagValue, pTag->szAddress, this->m_strFilePath.c_str(), m_nSheetNo + 1, nMatchRow + 1, vecRows.size());
		Drv_SetTagData_Text(pTag, szTagValue);
		vecTagToUpdate.push_back(pTag);
		continue;
	} // for (int i = 0; i < pDevice->nTagNum; i++)

	int nRet = Drv_UpdateTagsData(m_pDevice, vecTagToUpdate.data(), vecTagToUpdate.size());
	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, sheetNo:%d, file:%s, upateTagNum:%d, ret:%d", m_pDevice->szName, m_nSheetNo + 1, m_strFilePath.c_str(), vecTagToUpdate.size(), nRet);

	//释放内存
	vecTagToUpdate.clear();
	return 0;
}