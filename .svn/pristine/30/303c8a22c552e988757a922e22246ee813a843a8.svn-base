#pragma  once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <vector>	//vector头文件
#include <string>	//string
#include <map>		//map
#include <list>		//list
#include <time.h>
#include <fstream>
#include <sstream>
//excel 头文件 这边是在网上下载的库 然后会有头文件和lib库
#include "libxl/libxl.h"
#include "libxl/IFormatT.h"
#include "libxl/IBookT.h"
#include "libxl/IAutoFilterT.h"
#include "libxl/enum.h"
#include "libxl/IFilterColumnT.h"
#include "libxl/IFontT.h"
#include "libxl/setup.h"
#include "sys/stat.h"

#include "pkdbapi/pkdbapi.h"
#include "stdio.h"
#include "pkcomm/pkcomm.h"
#include "pkdriver/pkdrvcmn.h"

#define KEY "Halil Kural"
#define VAL "windows-2723210a07c4e90162b26966a8jcdboe"

using  namespace std;
using  namespace libxl;

#define	METHOD_UNKNOWN						0	// 无此方法
#define	METHOD_GETCELL_VALUE_XY				1	// 直接得到值的方法, Cell:B13
#define	METHOD_COLUMNVALUE					2	// 直接得到值的方法, Col(B);C=Now(YYYY/M/D),D=MaxText()
#define	METHOD_GETNOW_TIMEFORMAT			3	// 直接得到当前时间的方法, Now(YYYY-MM-DD)
#define METHOD_COUNT_MATCHCOL				4	// 计数的方法,Count;C=Now(YYYY/M/D),I=OK
#define METHOD_SUM_MATCHCOL					5	// 计数的方法,Sum(Q);C=Now(YYYY/M/D),I=OK

#define COL_MATCHMODE_EQUAL					1	// 目前仅支持相等1种匹配条件

#define COL_MATCH_FUNCTION_NONE				0	// 不是函数，直接列值是否和文本相等
#define COL_MATCH_FUNCTION_NOW				1	// 
#define COL_MATCH_FUNCTION_MAXTEXT			2	// MaxText函数，返回文本
#define COL_MATCH_FUNCTION_MAXINT			3	// MaxInt函数，返回数值int
#define COL_MATCH_FUNCTION_MAXREAL			4	// MaxReal函数，返回数值double
#define COL_MATCH_FUNCTION_MAXTIME			5	// MaxTime函数，返回数值time_t

#define MIN_INTEGER							-100000000
#define MIN_RELA							-100000000
extern int GetNowTimeString(string &strTime, const char *szTimeFormat);
extern string Trim(string& str);
int SplitFuncNameWithParam(PKDEVICE *pDevice, PKTAG *pTag, string strFuncWithParam, string &strFuncName, string &strParam);
void clearRows(vector<vector<string> >&rows);
// 统计列时的一个匹配条件，可以有多个匹配条件. MaxNow,MaxNumber,MaxText,Max
class CColStatMatch
{
public:
	PKDEVICE *m_pDevice;
	PKTAG * m_pTag;
	string	m_strColName;			// A,B...这里用不着，放这里是打印日志用
	int		m_nColIndex;			// 转换为0开始的索引号

	int		m_nMatchFuncNo;			// 匹配时所用的函数
	int		m_nMatchMode;			// 目前为包含
	string	m_strWhereConditions;	// 匹配上的值

	string	m_strTimeFormat;		// 匹配的时间格式，用于Now(YYYY-MM-DD),MaxTime(D,YYYY-MM-DD)的
	string  m_strTextToCompare;		// 比较的文本内容

	// 下面是临时变量
	string	m_strCurMaxText;		// 最大文本。用于MaxText()函数
	double  m_dbCurMaxReal;			// 最大数值。用于MaxReal()函数
	int		m_nCurMaxInt;			// 最大数值。用于MaxInt()函
	time_t	m_tmCurMaxTime;			// 最大时间。用于MaxTime(hh;mm;ss)函数

public:
	CColStatMatch();

public:
	int ParseMatchConditionInfo(string strWhereConds); // {条件表达式}:MaxText(D),MaxTime(D,hh;mm;ss),MaxInt(D),MaxReal(D)
	void ResetMatchTemp();
	bool Matched(const char *szColValue); // 是否匹配成功.如果是MaxInt/MaxReal/MaxText/MaxTime则和内部的m_strCurMaxtext比较，超过则算是匹配成功
};

class CRowCalcMethod
{
public:
	int		m_nCalcMethod;			// 计算方法
	PKDEVICE	*m_pDevice;
	PKTAG		*m_pTag;
	string	m_strNow_TimeFormat;	// 取现在时间时的格式
	int		m_nFuncParam_ColNo;	// 获取单元格时第Row行，Col列,Cell方法, SUM(A)
	int		m_nFuncParam_RowNo;	// 获取单元格时第Row行，Col列, Cell方法

	vector<CColStatMatch *>	m_vecColMatch;	// 统计Count时列的匹配条件
public:
	CRowCalcMethod(PKDEVICE *pDevice);

public:
	int ParseMethodSection(const char *szMethodSection); // 分析方法字段
};

class CExcelDevice
{
public:
	PKDEVICE *m_pDevice;
	string		m_strFilePath;
	string		m_strSheetNo;
	int			m_nSheetNo;
	bool		m_bRecursiveSubDir; // 是否循环查找子目录。0表示不查找子目录，为空或其他值表示查找子目录
public:
	CExcelDevice(PKDEVICE *pDevice);

public:
	time_t GetModifyTime(string strPath);

 	int ProcessExcelFile(const char *szFilePath);
	int ProcessCSVFile(const char *szFilePath);

protected:
	Book* InitAndCreateBook(string strPath);
	int ReadRowsExcelFile(const char *szFilePath, vector<vector<string> >& vecRows); // excel的N行N列;
	int ReadRowsCSVFile(const char *szFilePath, vector<vector<string> >& vecRows); // excel的N行N列;
	int CalcTagsFromRows(vector<vector<string> > &vecRows);

};
