#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Neoeter/Platform/PortTypes.h>

namespace Neoeter
{
	namespace Platform
	{
		static const uint32_t kPlatformInfinite = 0xFFFFFFFFu;

		class IPlatformPort
		{
			public:
				virtual ~IPlatformPort() = default;

				virtual uint64_t MonotonicMillis() = 0;
				virtual void SleepMillis(uint32_t milliseconds) = 0;
				virtual uint32_t TickCount() = 0;

				virtual bool SetEnvironment(const char* name, const char* value) = 0;
				virtual bool FileExists(const char* path) = 0;
				virtual bool RemoveFile(const char* path) = 0;
				virtual bool CreateDirectory(const char* path) = 0;
				virtual std::string CurrentDirectory() = 0;
				virtual std::vector<std::string> FindFiles(const char* pattern) = 0;

				virtual bool RaiseTimerResolution(uint32_t* periodMilliseconds) = 0;
				virtual void RestoreTimerResolution(uint32_t periodMilliseconds) = 0;

				virtual void ShowMessage(const char* title, const char* text) = 0;
				virtual int ToWide(const char* text, wchar_t* out, int capacity) = 0;
				virtual int FromWide(const wchar_t* text, char* out, int capacity) = 0;

				virtual bool InitializeCom() = 0;
				virtual void ShutdownCom() = 0;

				virtual PlatformModule ModuleHandle() = 0;
				virtual PlatformThreadHandle StartThread(uint32_t (*entry)(void*), void* arg) = 0;
				virtual bool JoinThread(PlatformThreadHandle thread, uint32_t timeoutMilliseconds) = 0;
				virtual void CloseThread(PlatformThreadHandle thread) = 0;
				virtual PlatformSemaphoreHandle CreateSemaphore(uint32_t initialCount, uint32_t maximumCount) = 0;
				virtual bool ReleaseSemaphore(PlatformSemaphoreHandle semaphore, uint32_t count) = 0;
				virtual bool WaitSemaphore(PlatformSemaphoreHandle semaphore, uint32_t timeoutMilliseconds) = 0;
				virtual void CloseSemaphore(PlatformSemaphoreHandle semaphore) = 0;
				virtual bool IsLinux() = 0;
				virtual int CompilerVersion() = 0;
				virtual void SetFloatingPointPrecision() = 0;
		};

		// The adapter for the current target. Callers use Platform_GetPort().
		IPlatformPort* CreatePlatformPort();

		IPlatformPort& Platform_GetPort();

		// Installs a test port. Passing null restores the default adapter.
		void Platform_SetPort(IPlatformPort* port);

		inline uint64_t MonotonicMillis()
		{
			return Platform_GetPort().MonotonicMillis();
		}

		inline void SleepMillis(uint32_t milliseconds)
		{
			Platform_GetPort().SleepMillis(milliseconds);
		}

		inline uint32_t TickCount()
		{
			return Platform_GetPort().TickCount();
		}

		inline bool SetEnvironment(const char* name, const char* value)
		{
			return Platform_GetPort().SetEnvironment(name, value);
		}

		inline bool FileExists(const char* path)
		{
			return Platform_GetPort().FileExists(path);
		}

		inline bool RemoveFile(const char* path)
		{
			return Platform_GetPort().RemoveFile(path);
		}

		inline bool CreateDirectory(const char* path)
		{
			return Platform_GetPort().CreateDirectory(path);
		}

		inline std::string CurrentDirectory()
		{
			return Platform_GetPort().CurrentDirectory();
		}

		inline std::vector<std::string> FindFiles(const char* pattern)
		{
			return Platform_GetPort().FindFiles(pattern);
		}

		inline bool RaiseTimerResolution(uint32_t* periodMilliseconds)
		{
			return Platform_GetPort().RaiseTimerResolution(periodMilliseconds);
		}

		inline void RestoreTimerResolution(uint32_t periodMilliseconds)
		{
			Platform_GetPort().RestoreTimerResolution(periodMilliseconds);
		}

		inline void ShowMessage(const char* title, const char* text)
		{
			Platform_GetPort().ShowMessage(title, text);
		}

		inline int ToWide(const char* text, wchar_t* out, int capacity)
		{
			return Platform_GetPort().ToWide(text, out, capacity);
		}

		inline int FromWide(const wchar_t* text, char* out, int capacity)
		{
			return Platform_GetPort().FromWide(text, out, capacity);
		}

		inline bool InitializeCom()
		{
			return Platform_GetPort().InitializeCom();
		}

		inline void ShutdownCom()
		{
			Platform_GetPort().ShutdownCom();
		}

		inline PlatformModule ModuleHandle()
		{
			return Platform_GetPort().ModuleHandle();
		}

		inline PlatformThreadHandle StartThread(uint32_t (*entry)(void*), void* arg)
		{
			return Platform_GetPort().StartThread(entry, arg);
		}

		inline bool JoinThread(PlatformThreadHandle thread, uint32_t timeoutMilliseconds)
		{
			return Platform_GetPort().JoinThread(thread, timeoutMilliseconds);
		}

		inline void CloseThread(PlatformThreadHandle thread)
		{
			Platform_GetPort().CloseThread(thread);
		}

		inline PlatformSemaphoreHandle CreateSemaphore(uint32_t initialCount, uint32_t maximumCount)
		{
			return Platform_GetPort().CreateSemaphore(initialCount, maximumCount);
		}

		inline bool ReleaseSemaphore(PlatformSemaphoreHandle semaphore, uint32_t count)
		{
			return Platform_GetPort().ReleaseSemaphore(semaphore, count);
		}

		inline bool WaitSemaphore(PlatformSemaphoreHandle semaphore, uint32_t timeoutMilliseconds)
		{
			return Platform_GetPort().WaitSemaphore(semaphore, timeoutMilliseconds);
		}

		inline void CloseSemaphore(PlatformSemaphoreHandle semaphore)
		{
			Platform_GetPort().CloseSemaphore(semaphore);
		}

		inline bool IsLinux()
		{
			return Platform_GetPort().IsLinux();
		}

		inline int CompilerVersion()
		{
			return Platform_GetPort().CompilerVersion();
		}

		inline void SetFloatingPointPrecision()
		{
			Platform_GetPort().SetFloatingPointPrecision();
		}
	}
}
