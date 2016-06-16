/**********************************************************

   *描    述：TCP客户端监听任务类 派生于ITaskProcess
   *作    用：为TCP客户端监听线程提供监听任务
   *注意事项：无

**********************************************************/

#ifndef _TASKTCPCLIENTLISTEN_H_
#define _TASKTCPCLIENTLISTEN_H_

#include "TcpSever.h"
#include "../FundationClass/Thread/ITaskProcess.h"

class TaskTcpClientListen : public ITaskProcess
{
public:
	TaskTcpClientListen(TcpServer *pTcpServer);

	//实现TCP客户端监听任务
	void TaskPorcess(void *pTask);

private:
	//TCP服务器类对象
	TcpServer *m_pTcpServer;
};

#endif