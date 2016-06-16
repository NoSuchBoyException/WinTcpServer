/**********************************************************

   *描    述：TCP数据处理抽象类
   *作    用：提供TCP数据处理接口
   *注意事项：需要向客户端返回处理结果时传入对应recvSocket

**********************************************************/

#ifndef _ITCPDATAPROCESS_H_
#define _ITCPDATAPROCESS_H_

#include <WinSock2.h>
#include <windows.h>

class ITcpDataProcess
{
public:
	virtual ~ITcpDataProcess(){}

	//TCP数据处理接口
	virtual void TcpDataProcess(const unsigned char *pTcpData, 
                                const int &iLen, 
                                const SOCKET &recvSocket = NULL) = 0;
};

#endif