#include "ThreadManager.h"

ThreadManager *ThreadManager::ms_pInstance = NULL;

ThreadManager *ThreadManager::GetInstance()
{
	if (NULL == ms_pInstance)
	{
		ms_pInstance = new ThreadManager;
	}

	return ms_pInstance;
}

ThreadManager::ThreadManager()
{
	/*
	  * 函数名：InitializeCriticalSectionAndSpinCount
	  * 功能：  结合旋转锁初始化临界区
	  * 参数：  lpCriticalSection    临界区地址
	  *         dwSpinCount          CPU时钟周期等待次数   一般为4000
	  *         lMaximumCount        允许最大资源计数
	  *         lpName               内核对象名称          可为NULL
	  * 返回值：总是返回一个非零值
	*/

	InitializeCriticalSectionAndSpinCount(&m_csThreadVector, 4000);
	InitializeCriticalSectionAndSpinCount(&m_csThreadIDQueue, 4000);

	/*
	  * 函数名：CreateSemaphore
	  * 功能：  创建并初始化信号量内核对象
	  * 参数：  lpSemaphoreAttributes   安全属性指针       一般为NULL
	  *         lInitialCount           初始资源计数
	  *         lMaximumCount           允许最大资源计数
	  *         lpName                  内核对象名称       可为NULL
	  * 返回值：成功时返回信号量内核对象句柄 失败时返回NULL
	*/
	m_semDeleteThread = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	assert(NULL != m_semDeleteThread);

	m_semExitManageThread = CreateSemaphore(NULL, 0, 1, NULL);
	assert(NULL != m_semExitManageThread);

	//初始化连接服务线程退出标识
	m_bIsExitThread = false;

	//创建用于删除连接服务线程的管理线程
	m_pManageThread = new Thread(ThreadManageFunc, this);
	m_pManageThread->Start();
}

ThreadManager::~ThreadManager()
{
	//等待所有连接服务线程删除完成
	while (0 != GetThreadCount())
	{
		Sleep(1);
	}

	//释放退出管理线程信号量信号
	ReleaseSemaphore(m_semExitManageThread, 1, NULL);
	//等待管理线程退出
	WaitForThreadExit(m_pManageThread->GetThreadHandle());
	//析构管理线程
	delete m_pManageThread;

	//释放内核对象 防止内核泄露
	CloseHandle(m_semDeleteThread);
	m_semDeleteThread = NULL;

	CloseHandle(m_semExitManageThread);
	m_semExitManageThread = NULL;
	
	DeleteCriticalSection(&m_csThreadVector);
	DeleteCriticalSection(&m_csThreadIDQueue);
}

void ThreadManager::AddThread(IThread *pThread)
{
	if (NULL == pThread)
	{
		return;
	}

	EnterCriticalSection(&m_csThreadVector);
	m_ThreadVector.push_back(pThread);
	cout << "Thread count is: " << m_ThreadVector.size() << endl;
	LeaveCriticalSection(&m_csThreadVector);
}

void ThreadManager::ExitThread()
{
	m_bIsExitThread = true;
}

bool ThreadManager::IsExitThread() const
{
	return m_bIsExitThread;
}

void ThreadManager::DeleteThread(const DWORD &dwThreadID)
{
	EnterCriticalSection(&m_csThreadIDQueue);
	m_ThreadIDQueue.push(dwThreadID);
	LeaveCriticalSection(&m_csThreadIDQueue);

	//释放删除连接服务线程信号量信号
	ReleaseSemaphore(m_semDeleteThread, 1, NULL);
}

size_t ThreadManager::GetThreadCount()
{
	EnterCriticalSection(&m_csThreadVector);
	size_t zThreadCount = m_ThreadVector.size();
	LeaveCriticalSection(&m_csThreadVector);

	return zThreadCount;
}

void ThreadManager::WaitForThreadExit(const HANDLE &hThreadHandle)
{
	DWORD dwThreadExitCode = 0;
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

void ThreadManager::ThreadManageFunc(void *pParam)
{
	if (NULL == pParam)
	{
		return;
	}

	ThreadManager *pThisManager = static_cast<ThreadManager *>(pParam);

	//信号等待数组
	HANDLE arrSingalWait[2] = { pThisManager->m_semDeleteThread, 
						        pThisManager->m_semExitManageThread };
	DWORD dwSingalValue = 0;
	DWORD dwThreadID = 0;
	bool bHasThreadID = false;
	bool bHasThread = false;
	while (true)
	{
		//等待arrSingalWait中的信号
		dwSingalValue = WaitForMultipleObjects(2, arrSingalWait, 
                                               false, INFINITE);
		//收到删除服务线程信号量信号
		if (WAIT_OBJECT_0 == dwSingalValue)  
		{
			//从连接服务线程ID队列取出待删除线程的ID
			EnterCriticalSection(&pThisManager->m_csThreadIDQueue);
			bHasThreadID = !pThisManager->m_ThreadIDQueue.empty();
			if (true == bHasThreadID)
			{
				dwThreadID = pThisManager->m_ThreadIDQueue.front();
				pThisManager->m_ThreadIDQueue.pop();
			}
			LeaveCriticalSection(&pThisManager->m_csThreadIDQueue);

			//从线程容器寻找并删除连接服务线程
			EnterCriticalSection(&pThisManager->m_csThreadVector);
			bHasThread = !pThisManager->m_ThreadVector.empty();
			if (true==bHasThreadID && true==bHasThread)
			{
				typedef vector<IThread *>::iterator ThreadIter;
				ThreadIter iter = pThisManager->m_ThreadVector.begin();
				for (; iter != pThisManager->m_ThreadVector.end(); ++iter)
				{
					if (dwThreadID == (*iter)->GetThreadID())
					{
						//等待该线程退出
						pThisManager->WaitForThreadExit((*iter)->GetThreadHandle());
						//析构该线程
						delete static_cast<IThread *>(*iter);
						//从线程单元容器擦除该线程
						pThisManager->m_ThreadVector.erase(iter);
						break;
					}
				}
			}
			LeaveCriticalSection(&pThisManager->m_csThreadVector);
		}
		//收到退出管理线程信号量信号
		else
		{
			break;
		}
	}
}