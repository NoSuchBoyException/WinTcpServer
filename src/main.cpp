#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "TcpServer/TaskTcpClientListen.h"
#include "TcpServer/TcpDataProcessAlgo.h"
#include "TcpServer/TcpSever.h"

//线程退出等待函数
void WaitForThreadExit(const HANDLE &hThreadHandle);

int main()
{
	//初始内存状态记录
	_CrtMemState s1, s2, s3;
	_CrtMemCheckpoint(&s1);

	//具体的TCP数据处理策略
	ITcpDataProcess *pTcpDataProcess = new TcpDataProcessAlgo;

	//针对多客户端的TCP服务器
	TcpServer *pTcpServer = new TcpServer(pTcpDataProcess);

	//设定通信端口并建立TCP客户端监听任务
	int iPort = 32888;
	ITaskProcess *pTaskClientListen = new TaskTcpClientListen(pTcpServer);

	//创建TCP客户端监听子线程
	IThread *pClientListenThread = new Thread(pTaskClientListen, (void *)iPort);
	pClientListenThread->Start();
	
	getchar();

 	pTcpServer->StopListen();

	//等待TCP客户端监听线程退出
	WaitForThreadExit(pClientListenThread->GetThreadHandle());
	//析构TCP客户端监听线程
	delete pClientListenThread;

	delete pTaskClientListen;
	delete pTcpServer;
	delete pTcpDataProcess;
	
	//最终内存状态记录
	_CrtMemCheckpoint(&s2);
	//内存泄露检测
	if (0 != _CrtMemDifference(&s3, &s1, &s2))
	{
		_CrtMemDumpStatistics(&s3);
	}

	return 0;
}

void WaitForThreadExit(const HANDLE &hThreadHandle)
{
	DWORD dwThreadExitCode;
	while (true)
	{
		//利用线程退出码判断线程是否已退出
		if (TRUE == GetExitCodeThread(hThreadHandle, &dwThreadExitCode))
		{
			if (STILL_ACTIVE == dwThreadExitCode)
			{
				Sleep(1);
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			Sleep(1);
			continue;
		}
	}
}