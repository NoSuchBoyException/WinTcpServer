#include "TcpSever.h"

TcpServer::TcpServer(ITcpDataProcess *pTcpDataProcess)
{
	m_bIsStopListen = false;

	//TCP客户端监听任务
	m_pTaskProcess = new TaskTcpDataRecv(pTcpDataProcess);

	//获取连接服务线程管理器单例
	m_pThreadManager = ThreadManager::GetInstance();

	//初始化Socket库WS2_32.dll
	InitSocketLib();
}

TcpServer::~TcpServer()
{
	//关闭客户端监听Socket
	if (INVALID_SOCKET != m_socketListen)
	{
		closesocket(m_socketListen);
	}

	//解除与Socket库WS2_32.dll的绑定并释放资源
	WSACleanup();

	//析构连接服务线程管理器单例
	delete m_pThreadManager;

	//析构TCP客户端监听任务
	delete m_pTaskProcess;
}

void TcpServer::StartListen(const int &iPort)
{
	if (0 > iPort)
	{
		return;
	}

	//初始化客户端监听Socket
	m_socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_socketListen)
	{
		cout << "socket() failed with error: " << WSAGetLastError();

		return;
	}

	//绑定服务器端地址信息
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(iPort);
	m_addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (SOCKET_ERROR == bind(m_socketListen, (SOCKADDR *)&m_addrServer, 
                             sizeof(SOCKADDR)))
	{
		cout << "bind() failed with error: " << WSAGetLastError();

		return;
	}

	//设置Socket接收模式为非阻塞（ulMode: 为0阻塞模式 !0为非阻塞模式）
	u_long ulMode = 1;
	if (SOCKET_ERROR == ioctlsocket(m_socketListen, FIONBIO, &ulMode))
	{
		cout << "Set accept() non-block mode failed with error: " 
			<<  WSAGetLastError();

		return;
	}

	//开始监听客户端连接
	listen(m_socketListen, MAX_CLIENT_LISTEN_NUM);

	if (NULL!=m_pThreadManager && NULL!=m_pTaskProcess)
	{
		//循环接收客户端连接
		int iAddrLen = sizeof(m_addrClient);
		IThread *pServiceThread = NULL;
		while (false == m_bIsStopListen)
		{
			//接收客户端连接
			SOCKET newSocket = accept(m_socketListen, 
                                     (SOCKADDR *)&m_addrClient, 
									  &iAddrLen);
			//没有新连接到来
			if (SOCKET_ERROR == newSocket)
			{
				Sleep(1);
				continue;
			}
			//新连接到来
			else
			{
				//把新连接的Socket交给子线程处理
				pServiceThread = new Thread(m_pTaskProcess, (void *)newSocket);
				pServiceThread->Start();

				//将该连接服务线程放入线程管理器
				m_pThreadManager->AddThread(pServiceThread);
			}
		}
	}
}

void TcpServer::StopListen()
{
	//停止监听客户端连接
	m_bIsStopListen = true;

	//退出已创建的连接服务线程
	m_pThreadManager->ExitThread();
}

bool TcpServer::SendData(const char *pTcpData, const int &iLen, 
	                     const SOCKET &recvSocket)
{
	if (NULL==pTcpData || 0>=iLen || NULL==recvSocket || 
		INVALID_SOCKET==recvSocket)
	{
		return false;
	}

	//发送数据
	send(recvSocket, pTcpData, iLen, 0);

	return true;
}

void TcpServer::InitSocketLib(const BYTE &byMinorVersion, 
                              const BYTE &byMajorVersion)
{
	WSADATA wsaData;
	WORD wSockVersion = MAKEWORD(byMinorVersion, byMajorVersion);

	//初始化Socket库WS2_32.dll
	if (0 != WSAStartup(wSockVersion, &wsaData))
	{
		exit(0);
	}
}