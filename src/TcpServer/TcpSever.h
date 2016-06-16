/**********************************************************

   *描    述：TCP服务器类
   *作    用：为多TCP客户端提供服务
   *注意事项：1.最大TCP客户端监听数为64
              2.加入了心跳机制，能够检测并处理网线断开、客
			    户端进程异常、客户端主机宕机或断电等情况

**********************************************************/

#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#pragma comment(lib, "WS2_32")

#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include "ITcpDataProcess.h"
#include "TaskTcpDataRecv.h"
#include "ThreadManager.h"
#include "../FundationClass/Thread/Thread.h"

using namespace std;

class TcpServer
{
public:
	TcpServer(ITcpDataProcess *pTcpDataProcess);
	~TcpServer();

	//开始监听客户端
	void StartListen(const int &iPort);

	//停止监听客户端
	void StopListen();

	//向客户端发送数据
	static bool SendData(const char *pTcpData, 
                         const int &iLen, 
                         const SOCKET &recvSocket);

private:
	//初始化Socket库WS2_32.dll
	void InitSocketLib(const BYTE &byMinorVersion = 2, 
                       const BYTE &byMajorVersion = 2);

private:
	//客户端监听Socket
	SOCKET m_socketListen;

	//服务器端和客户端地址信息
	sockaddr_in m_addrServer;
	sockaddr_in m_addrClient;

	//客户端监听停止标识
	bool m_bIsStopListen;

	//TCP数据获取任务对象
	ITaskProcess *m_pTaskProcess;

	//连接服务线程管理器单例
	ThreadManager *m_pThreadManager;

	//最大客户端监听数
	static const int MAX_CLIENT_LISTEN_NUM = 64;
};

#endif