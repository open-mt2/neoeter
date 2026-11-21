#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/IO/EterPackManager.h>
#include <Neoeter/Platform/Port.h>
#include <Neoeter/Render/FileLoaderThread.h>
#include <Neoeter/Render/ResourceManager.h>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

CFileLoaderThread::CFileLoaderThread() : m_hThread(NULL), m_pArg(NULL), m_hSemaphore(NULL), m_bShutdowned(false)
{
}

CFileLoaderThread::~CFileLoaderThread()
{
	Shutdown();
	Destroy();
}

int CFileLoaderThread::Create(void* arg)
{
	if (m_hThread != NULL)
		return false;

	m_hSemaphore = Neoeter::Platform::CreateSemaphore(0, 65535);
	if (m_hSemaphore == NULL)
		return false;

	Arg(arg);
	m_hThread = Neoeter::Platform::StartThread(&CFileLoaderThread::EntryPoint, this);
	if (m_hThread == NULL)
	{
		Neoeter::Platform::CloseSemaphore(m_hSemaphore);
		m_hSemaphore = NULL;
		return false;
	}

	return true;
}

uint32_t CFileLoaderThread::Run(void* arg)
{
	return (Execute(arg));
}

/* Static */
uint32_t CFileLoaderThread::EntryPoint(void* pThis)
{
	CFileLoaderThread* pThread = (CFileLoaderThread*)pThis;
	return pThread->Run(pThread->Arg());
}

//////////////////////////////////////////////////////////////////////////
void CFileLoaderThread::Destroy()
{
}

void CFileLoaderThread::Shutdown()
{
	if (m_bShutdowned.exchange(true))
		return;

	if (m_hSemaphore != NULL)
		Neoeter::Platform::ReleaseSemaphore(m_hSemaphore, 1);

	bool joined = true;
	if (m_hThread != NULL)
	{
		joined = Neoeter::Platform::JoinThread(m_hThread, 10000);
		Neoeter::Platform::CloseThread(m_hThread);
		m_hThread = NULL;
	}

	if (joined && m_hSemaphore != NULL)
	{
		Neoeter::Platform::CloseSemaphore(m_hSemaphore);
		m_hSemaphore = NULL;
	}
}

uint32_t CFileLoaderThread::Execute(void* /*pvArg*/)
{
	while (!m_bShutdowned.load())
	{
		if (!Neoeter::Platform::WaitSemaphore(m_hSemaphore, Neoeter::Platform::kPlatformInfinite))
			break;

		if (m_bShutdowned.load())
			break;

		Process();
	}

	Destroy();
	return 1;
}

void CFileLoaderThread::Request(const std::string& c_rstFileName) // called in main thread
{
	TData pData;
	pData.fileName = c_rstFileName;
	pData.data = nullptr;

	m_RequestMutex.Lock();
	m_pRequestDeque.push_back(pData);
	m_RequestMutex.Unlock();

	if (!Neoeter::Platform::ReleaseSemaphore(m_hSemaphore, 1))
		TraceError("CFileLoaderThread::Request: ReleaseSemaphore error");
}

bool CFileLoaderThread::Fetch(TData& data) // called in main thread
{
	m_CompleteMutex.Lock();

	if (m_pCompleteDeque.empty())
	{
		m_CompleteMutex.Unlock();
		return false;
	}

	data = m_pCompleteDeque.front();
	m_pCompleteDeque.pop_front();

	m_CompleteMutex.Unlock();
	return true;
}

void CFileLoaderThread::Process() // called in loader thread
{
	m_RequestMutex.Lock();

	if (m_pRequestDeque.empty())
	{
		m_RequestMutex.Unlock();
		return;
	}

	auto request = m_pRequestDeque.front();
	m_pRequestDeque.pop_front();

	m_RequestMutex.Unlock();

	if (!CEterPackManager::Instance().Get(request.fileName, request.data))
		request.data = nullptr;

	m_CompleteMutex.Lock();
	m_pCompleteDeque.push_back(request);
	m_CompleteMutex.Unlock();

	Neoeter::Platform::SleepMillis(g_iLoadingDelayTime);
}
