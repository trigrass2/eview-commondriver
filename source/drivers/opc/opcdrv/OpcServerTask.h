/**************************************************************
 *  Filename:    DeviceScanTask.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: �豸���ݿ�ɨ����.
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
	// �豸���ݿ�ɨ��
	long ScanAllDevice();

	// �������ݿ飬������ɨ�����ڣ������ɶ�����
	long ReadOPCGroup(COPCGroup* pGroup);
	
private:
	COPCServer	*m_pOPCServer;	// �����豸��ָ��
};

#endif  // _DEVICE_SCAN_TASK_H_
