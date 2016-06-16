#include "TaskTcpDataRecv.h"

TaskTcpDataRecv::TaskTcpDataRecv(ITcpDataProcess *pTcpDataProcess)
{
	m_pTcpDataProcess = pTcpDataProcess;

	m_pThreadManager = ThreadManager::GetInstance();
}

void TaskTcpDataRecv::TaskPorcess(void *pTask)
{
	if (NULL == pTask)
	{
		return;
	}

	SOCKET recvSocket = (SOCKET)pTask;
	if (NULL==recvSocket || INVALID_SOCKET==recvSocket)
	{
		cout << "Pass a invalid socket to service thread";

		return;
	}

	//设置数据接收模式为非阻塞（ulMode: 0为阻塞模式 !0为非阻塞模式）
	u_long ulMode = 1;
	if (SOCKET_ERROR == ioctlsocket(recvSocket, FIONBIO, &ulMode))
	{
		cout << "Set recv() not-block mode failed with error: "
			 <<  WSAGetLastError();

		closesocket(recvSocket);

		return;
	}

	//用于检测TCP连接是否异常断开的定时器
	tcp_keepalive inKeepAlive = {0};
	DWORD dwInLen = sizeof(tcp_keepalive);

	tcp_keepalive outKeepAlive = {0};
	DWORD dwOutLen = sizeof(tcp_keepalive);

	//是否启用tcp_keepalive定时器 1为启用
	inKeepAlive.onoff = 1;
	//首次keepalive探测前的等待时间（单位:ms）
	inKeepAlive.keepalivetime = 30000;
	//每次keepalive探测的时间间隔（单位:ms）
	inKeepAlive.keepaliveinterval = 3000;

	DWORD dwBytesReturn = 0;
	//开启检测TCP连接是否异常断开的tcp_keepalive定时器
	if(SOCKET_ERROR == WSAIoctl(recvSocket, SIO_KEEPALIVE_VALS, 
                                (LPVOID)&inKeepAlive, dwInLen, 
							    (LPVOID)&outKeepAlive, dwOutLen, 
							    &dwBytesReturn, NULL, NULL))
	{
		cout << "Set keep alive TCP Connection failed with error " 
             <<  WSAGetLastError();

		closesocket(recvSocket);

		return;
	}

	if (NULL!=m_pThreadManager && NULL!=m_pTcpDataProcess)
	{
		//循环接收数据
		int iRecvValue = -1;
		unsigned char buffer[MAX_BUFFER_READ_LENGTH];
		memset(buffer, 0, MAX_BUFFER_READ_LENGTH);
		while (false == m_pThreadManager->IsExitThread())
		{
			iRecvValue = recv(recvSocket, (char *)buffer, 
                              MAX_BUFFER_READ_LENGTH, 0);
			//本次循环没有数据到来或发生连接异常
			if (SOCKET_ERROR == iRecvValue)
			{
				//网线断开、客户端进程异常等导致连接异常
				if (WSAECONNRESET == WSAGetLastError())
				{
					break;
				}
				//没有数据到来 继续循环接收数据
				else
				{
					Sleep(1);
					continue;
				}
			}
			//收到并处理客户端发来的数据
			else if (0 < iRecvValue)
			{
				m_pTcpDataProcess->TcpDataProcess((unsigned char *)buffer, 
                                                   iRecvValue, recvSocket);
			}
			//客户端主机宕机或断电等导致连接异常
			else if (0 == iRecvValue )
			{
				break;
			}
		}
	}
	
	closesocket(recvSocket);

	//从连接服务线程管理器中删除自己
	cout << "Thread will be deleted is:" << GetCurrentThreadId() << endl;
	m_pThreadManager->DeleteThread(GetCurrentThreadId());
}