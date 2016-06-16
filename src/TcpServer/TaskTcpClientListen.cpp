#include "TaskTcpClientListen.h"

TaskTcpClientListen::TaskTcpClientListen(TcpServer *pTcpServer)
{
	m_pTcpServer = pTcpServer;
}

void TaskTcpClientListen::TaskPorcess(void *pTask)
{
	if (NULL == pTask)
	{
		return;
	}

	int iPort = (int)pTask;
	if (NULL != m_pTcpServer)
	{
		//开始监听客户端
		m_pTcpServer->StartListen(iPort);
	}
}