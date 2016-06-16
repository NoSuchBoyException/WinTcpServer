/**********************************************************

   *描    述：TCP数据获取任务类 派生于ITaskProcess
   *作    用：实现TCP数据获取任务
   *注意事项：TCP数据缓冲区最大读取长度为1024字节

**********************************************************/

#ifndef _TASKTCPDATARECV_H_
#define _TASKTCPDATARECV_H_

#include <WinSock2.h>
#include <mstcpip.h>
#include <windows.h>
#include <iostream>
#include "ITcpDataProcess.h"
#include "ThreadManager.h"
#include "../FundationClass/Thread/ITaskProcess.h"

using namespace std;

class TaskTcpDataRecv : public ITaskProcess
{
public:
	TaskTcpDataRecv(ITcpDataProcess *pTcpDataProcess);

	//实现TCP数据获取任务
	virtual void TaskPorcess(void *pTask);

private:
	//TCP数据处理对象
	ITcpDataProcess *m_pTcpDataProcess;

	//连接服务线程管理器单例
	ThreadManager *m_pThreadManager;

	//TCP数据缓冲区最大读取长度
	static const int MAX_BUFFER_READ_LENGTH = 1024;
};

#endif