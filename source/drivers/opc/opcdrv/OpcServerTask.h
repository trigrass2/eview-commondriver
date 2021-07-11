/**************************************************************
 *  Filename:    DeviceScanTask.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备数据块扫描类.
 *
 *  @author:     lijingjing
 *  @version     05/28/2008  lijingjing  Initial Version
**************************************************************/

#ifndef _DEVICE_SCAN_TASK_H_
#define _DEVICE_SCAN_TASK_H_

#include <ace/Task.h>
#include <string>
#include <map>

#include "CommHelper.h"
using namespace  std;

class COPCServer;
class COPCGroup;
class COPCServerTask : public ACE_Task<ACE_MT_SYNCH>
{
public:
	COPCServerTask(COPCServer* pOPCServer);
	virtual ~COPCServerTask();
	
	int Start();
	void Stop();

protected:
	bool			m_bStop;
	bool			m_bStopped;

private:
	virtual int svc();
	// 设备数据块扫描
	long ScanAllDevice();

	// 处理数据块，若到达扫描周期，则生成读命令
	long ReadOPCGroup(COPCGroup* pGroup);
	
private:
	COPCServer	*m_pOPCServer;	// 所属设备组指针
};

#endif  // _DEVICE_SCAN_TASK_H_
