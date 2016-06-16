#include "TcpDataProcessAlgo.h"

void TcpDataProcessAlgo::TcpDataProcess(const unsigned char *pTcpData, 
                                        const int &iLen, 
										const SOCKET &recvSocket)
{
	if (NULL==pTcpData || 0>=iLen)
	{
		return;
	}

	//模拟算法处理
	Sleep(50);

	if (NULL!=recvSocket && INVALID_SOCKET!=recvSocket)
	{
		//向客户端返回处理结果
		if (true == TcpServer::SendData((char *)pTcpData, iLen, recvSocket))
		{
			cout << "Data: " << pTcpData << " has been processed" << endl;
		}
	}
}