/**********************************************************

   *描    述：TCP数据处理实现类 派生于ITcpDataProcess
   *作    用：实现具体的TCP数据处理
   *注意事项：可重载构造函数以传入数据处理所需的对象或参数

**********************************************************/

#ifndef _TCPDATAPROCESSALGO_H_
#define _TCPDATAPROCESSALGO_H_

#include <iostream>
#include "TcpSever.h"
#include "ITcpDataProcess.h"

using namespace std;

class TcpDataProcessAlgo : public ITcpDataProcess
{
public:
	//实现具体的TCP数据处理
	virtual void TcpDataProcess(const unsigned char *pTcpData, 
                                const int &iLen, 
                                const SOCKET &recvSocket = NULL);
};

#endif