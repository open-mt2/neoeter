#ifndef __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__
#define __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__

#include <deque>
#include <memory>
#include <vector>
#include "Thread.h"
#include "Mutex.h"

class CFileLoaderThread
{
	public:
		typedef struct SData
		{
				std::string fileName;
				std::shared_ptr<std::vector<char>> data;
		} TData;

	public:
		CFileLoaderThread();
		~CFileLoaderThread();

		int Create(void* arg);

	public:
		void Request(const std::string& c_rstFileName);
		bool Fetch(TData& data);
		void Shutdown();

	protected:
		static uint32_t CALLBACK EntryPoint(void* pThis);
		uint32_t Run(void* arg);

		void* Arg() const { return m_pArg; }
		void Arg(void* arg) { m_pArg = arg; }

		PlatformHandle m_hThread;

	private:
		void* m_pArg;
		unsigned m_uThreadID;

	protected:
		uint32_t Setup();
		uint32_t Execute(void* pvArg);
		void Destroy();
		void Process();

	private:
		std::deque<TData> m_pRequestDeque;
		Mutex m_RequestMutex;

		std::deque<TData> m_pCompleteDeque;
		Mutex m_CompleteMutex;

		PlatformHandle m_hSemaphore;
		int m_iRestSemCount;
		bool m_bShutdowned;
};

#endif