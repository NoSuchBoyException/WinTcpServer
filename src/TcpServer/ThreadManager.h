/**********************************************************

   *描    述：线程管理器
   *作    用：实现对TCP连接服务线程的统一管理
   *注意事项：本管理器通过单例模式实现

**********************************************************/

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <windows.h>
#include <queue>
#include <vector>
#include <iostream>
#include <assert.h>
#include "../FundationClass/Thread/Thread.h"

using namespace std;

class ThreadManager
{
public:
	//调用线程管理器的唯一方法
	static ThreadManager *GetInstance();

	~ThreadManager();

	//向线程容器添加连接服务线程
	void AddThread(IThread *pThread);

	//退出连接服务线程
	void ExitThread(); 

	//是否退出连接服务线程
	bool IsExitThread() const;

	//从线程容器中删除连接服务线程
	void DeleteThread(const DWORD &dwThreadID);

	//获取连接服务线程总数
	size_t GetThreadCount();

private:
	ThreadManager();
	
	//线程退出等待函数
	void WaitForThreadExit(const HANDLE &hThreadHandle);

	//连接服务线程管理函数
	static void ThreadManageFunc(void *pParam = NULL);

private:
	//连接服务线程管理器单例
	static ThreadManager *ms_pInstance;

	//连接服务线程退出标识
	bool m_bIsExitThread;

	//管理线程 用于删除连接服务线程
	IThread *m_pManageThread;

	//删除连接服务线程信号量
	HANDLE m_semDeleteThread; 

	//退出管理线程信号量
	HANDLE m_semExitManageThread;

	//连接服务线程容器及其临界区
	vector<IThread *> m_ThreadVector;
	CRITICAL_SECTION m_csThreadVector;

	//连接服务线程ID队列及其临界区
	queue<DWORD> m_ThreadIDQueue;
	CRITICAL_SECTION m_csThreadIDQueue;
};

#endif