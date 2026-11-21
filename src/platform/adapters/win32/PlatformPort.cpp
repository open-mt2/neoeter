#include <Neoeter/Platform/Port.h>

#include <objbase.h>
#include <windows.h>
#include <mmsystem.h>

#include <cfloat>
#include <cwchar>
#include <filesystem>
#include <string>

namespace
{
	struct Win32ThreadStart
	{
			uint32_t (*entry)(void*);
			void* arg;
	};

	unsigned __stdcall Win32ThreadEntry(void* parameter)
	{
		Win32ThreadStart* start = static_cast<Win32ThreadStart*>(parameter);
		const uint32_t result = start->entry(start->arg);
		delete start;
		return result;
	}

	class Win32Port final : public Neoeter::Platform::IPlatformPort
	{
		public:
			uint64_t MonotonicMillis() override
			{
				return static_cast<uint64_t>(GetTickCount());
			}

			void SleepMillis(uint32_t milliseconds) override
			{
				Sleep(milliseconds);
			}

			uint32_t TickCount() override
			{
				return static_cast<uint32_t>(GetTickCount());
			}

			bool SetEnvironment(const char* name, const char* value) override
			{
				return name != nullptr && SetEnvironmentVariableA(name, value != nullptr ? value : "") != 0;
			}

			bool FileExists(const char* path) override
			{
				return path != nullptr && GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
			}

			bool RemoveFile(const char* path) override
			{
				return path != nullptr && DeleteFileA(path) != 0;
			}

			bool CreateDirectory(const char* path) override
			{
				if (path == nullptr)
					return false;

				if (::CreateDirectoryA(path, nullptr) != 0)
					return true;

				return GetLastError() == ERROR_ALREADY_EXISTS;
			}

			std::string CurrentDirectory() override
			{
				char buffer[MAX_PATH] = {};
				const DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
				if (length == 0 || length >= MAX_PATH)
					return std::string();

				return std::string(buffer, length);
			}

			std::vector<std::string> FindFiles(const char* pattern) override
			{
				std::vector<std::string> files;
				if (pattern == nullptr)
					return files;

				WIN32_FIND_DATAA data = {};
				HANDLE find = FindFirstFileA(pattern, &data);
				if (find == INVALID_HANDLE_VALUE)
					return files;

				do
				{
					files.push_back(data.cFileName);
				} while (FindNextFileA(find, &data));

				FindClose(find);
				return files;
			}

			bool RaiseTimerResolution(uint32_t* periodMilliseconds) override
			{
				TIMECAPS caps = {};
				if (timeGetDevCaps(&caps, sizeof(caps)) != TIMERR_NOERROR)
					return false;

				uint32_t period = caps.wPeriodMin < 1 ? 1 : caps.wPeriodMin;
				if (period > caps.wPeriodMax)
					period = caps.wPeriodMax;

				if (timeBeginPeriod(period) != TIMERR_NOERROR)
					return false;

				if (periodMilliseconds != nullptr)
					*periodMilliseconds = period;

				return true;
			}

			void RestoreTimerResolution(uint32_t periodMilliseconds) override
			{
				timeEndPeriod(periodMilliseconds);
			}

			void ShowMessage(const char* title, const char* text) override
			{
				MessageBoxA(nullptr, text != nullptr ? text : "", title != nullptr ? title : "", MB_OK | MB_ICONERROR);
			}

			int ToWide(const char* text, wchar_t* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity <= 0)
					return 0;

				const int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, out, capacity);
				return length > 0 ? length - 1 : 0;
			}

			int FromWide(const wchar_t* text, char* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity <= 0)
					return 0;

				const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, out, capacity, nullptr, nullptr);
				return length > 0 ? length - 1 : 0;
			}

			bool InitializeCom() override
			{
				const HRESULT result = CoInitialize(nullptr);
				return SUCCEEDED(result);
			}

			void ShutdownCom() override
			{
				CoUninitialize();
			}

			PlatformModule ModuleHandle() override
			{
				return GetModuleHandleA(nullptr);
			}

			PlatformThreadHandle StartThread(uint32_t (*entry)(void*), void* arg) override
			{
				Win32ThreadStart* start = new Win32ThreadStart{entry, arg};
				const uintptr_t handle = _beginthreadex(nullptr, 0, Win32ThreadEntry, start, 0, nullptr);
				if (handle == 0)
				{
					delete start;
					return nullptr;
				}

				return reinterpret_cast<PlatformThreadHandle>(handle);
			}

			bool JoinThread(PlatformThreadHandle thread, uint32_t timeoutMilliseconds) override
			{
				return WaitForSingleObject(thread, timeoutMilliseconds) == WAIT_OBJECT_0;
			}

			void CloseThread(PlatformThreadHandle thread) override
			{
				CloseHandle(thread);
			}

			PlatformSemaphoreHandle CreateSemaphore(uint32_t initialCount, uint32_t maximumCount) override
			{
				return CreateSemaphoreA(nullptr, static_cast<LONG>(initialCount), static_cast<LONG>(maximumCount), nullptr);
			}

			bool ReleaseSemaphore(PlatformSemaphoreHandle semaphore, uint32_t count) override
			{
				return ::ReleaseSemaphore(semaphore, static_cast<LONG>(count), nullptr) != 0;
			}

			bool WaitSemaphore(PlatformSemaphoreHandle semaphore, uint32_t timeoutMilliseconds) override
			{
				return WaitForSingleObject(semaphore, timeoutMilliseconds) == WAIT_OBJECT_0;
			}

			void CloseSemaphore(PlatformSemaphoreHandle semaphore) override
			{
				CloseHandle(semaphore);
			}

			bool IsLinux() override
			{
				return false;
			}

			int CompilerVersion() override
			{
#if defined(_MSC_VER)
				return _MSC_VER;
#else
				return 1900;
#endif
			}

			void SetFloatingPointPrecision() override
			{
#if defined(_MSC_VER) || defined(__MINGW32__)
				_controlfp(_PC_24, _MCW_PC);
#endif
			}
	};
}

namespace Neoeter
{
	namespace Platform
	{
		IPlatformPort* CreatePlatformPort()
		{
			return new Win32Port();
		}
	}
}
