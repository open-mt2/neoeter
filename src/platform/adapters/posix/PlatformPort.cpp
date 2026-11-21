#include <Neoeter/Platform/Port.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <mutex>
#include <thread>

#include <glob.h>
#include <unistd.h>

namespace
{
	struct PosixThread
	{
			std::thread thread;
			std::mutex mutex;
			std::condition_variable finished;
			bool done;
	};

	struct PosixSemaphore
	{
			std::mutex mutex;
			std::condition_variable available;
			uint32_t count;
			uint32_t maximum;
	};

	class PosixPort final : public Neoeter::Platform::IPlatformPort
	{
		public:
			uint64_t MonotonicMillis() override
			{
				const auto now = std::chrono::steady_clock::now().time_since_epoch();
				return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
			}

			void SleepMillis(uint32_t milliseconds) override
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
			}

			uint32_t TickCount() override
			{
				return static_cast<uint32_t>(MonotonicMillis());
			}

			bool SetEnvironment(const char* name, const char* value) override
			{
				return name != nullptr && setenv(name, value != nullptr ? value : "", 1) == 0;
			}

			bool FileExists(const char* path) override
			{
				if (path == nullptr)
					return false;

				std::error_code error;
				return std::filesystem::exists(path, error);
			}

			bool RemoveFile(const char* path) override
			{
				return path != nullptr && std::remove(path) == 0;
			}

			bool CreateDirectory(const char* path) override
			{
				if (path == nullptr)
					return false;

				std::error_code error;
				return std::filesystem::create_directories(path, error) || std::filesystem::is_directory(path, error);
			}

			std::string CurrentDirectory() override
			{
				std::error_code error;
				const std::filesystem::path path = std::filesystem::current_path(error);
				return error ? std::string() : path.string();
			}

			std::vector<std::string> FindFiles(const char* pattern) override
			{
				std::vector<std::string> files;
				if (pattern == nullptr)
					return files;

				glob_t matches = {};
				if (glob(pattern, 0, nullptr, &matches) != 0)
					return files;

				for (size_t i = 0; i < matches.gl_pathc; ++i)
					files.push_back(std::filesystem::path(matches.gl_pathv[i]).filename().string());

				globfree(&matches);
				return files;
			}

			bool RaiseTimerResolution(uint32_t* periodMilliseconds) override
			{
				if (periodMilliseconds != nullptr)
					*periodMilliseconds = 1;

				return true;
			}

			void RestoreTimerResolution(uint32_t) override
			{
			}

			void ShowMessage(const char* title, const char* text) override
			{
				std::fprintf(stderr, "%s: %s\n", title != nullptr ? title : "", text != nullptr ? text : "");
			}

			int ToWide(const char* text, wchar_t* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity <= 0)
					return 0;

				std::mbstate_t state = std::mbstate_t();
				const size_t length = std::mbsrtowcs(out, &text, static_cast<size_t>(capacity), &state);
				return length == static_cast<size_t>(-1) ? 0 : static_cast<int>(length);
			}

			int FromWide(const wchar_t* text, char* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity <= 0)
					return 0;

				std::mbstate_t state = std::mbstate_t();
				const size_t length = std::wcsrtombs(out, &text, static_cast<size_t>(capacity), &state);
				return length == static_cast<size_t>(-1) ? 0 : static_cast<int>(length);
			}

			bool InitializeCom() override
			{
				return true;
			}

			void ShutdownCom() override
			{
			}

			PlatformModule ModuleHandle() override
			{
				return nullptr;
			}

			PlatformThreadHandle StartThread(uint32_t (*entry)(void*), void* arg) override
			{
				PosixThread* thread = new PosixThread();
				thread->thread = std::thread([thread, entry, arg]
											 {
					entry(arg);
					{
						std::lock_guard<std::mutex> lock(thread->mutex);
						thread->done = true;
					}
					thread->finished.notify_all(); });
				return thread;
			}

			bool JoinThread(PlatformThreadHandle handle, uint32_t timeoutMilliseconds) override
			{
				PosixThread* thread = static_cast<PosixThread*>(handle);
				if (thread == nullptr)
					return false;

				std::unique_lock<std::mutex> lock(thread->mutex);
				bool finished = true;
				if (timeoutMilliseconds == Neoeter::Platform::kPlatformInfinite)
					thread->finished.wait(lock, [thread]
										  { return thread->done; });
				else
					finished = thread->finished.wait_for(lock, std::chrono::milliseconds(timeoutMilliseconds),
														 [thread]
														 { return thread->done; });
				lock.unlock();

				if (!finished)
					return false;

				if (thread->thread.joinable())
					thread->thread.join();

				return true;
			}

			void CloseThread(PlatformThreadHandle handle) override
			{
				PosixThread* thread = static_cast<PosixThread*>(handle);
				if (thread == nullptr)
					return;

				bool done = false;
				{
					std::lock_guard<std::mutex> lock(thread->mutex);
					done = thread->done;
				}

				if (thread->thread.joinable())
					thread->thread.detach();

				if (done)
					delete thread;
			}

			PlatformSemaphoreHandle CreateSemaphore(uint32_t initialCount, uint32_t maximumCount) override
			{
				PosixSemaphore* semaphore = new PosixSemaphore();
				semaphore->count = initialCount;
				semaphore->maximum = maximumCount;
				return semaphore;
			}

			bool ReleaseSemaphore(PlatformSemaphoreHandle handle, uint32_t count) override
			{
				PosixSemaphore* semaphore = static_cast<PosixSemaphore*>(handle);
				if (semaphore == nullptr)
					return false;

				std::lock_guard<std::mutex> lock(semaphore->mutex);
				if (semaphore->count + count > semaphore->maximum)
					return false;

				semaphore->count += count;
				semaphore->available.notify_all();
				return true;
			}

			bool WaitSemaphore(PlatformSemaphoreHandle handle, uint32_t timeoutMilliseconds) override
			{
				PosixSemaphore* semaphore = static_cast<PosixSemaphore*>(handle);
				if (semaphore == nullptr)
					return false;

				std::unique_lock<std::mutex> lock(semaphore->mutex);
				if (timeoutMilliseconds == Neoeter::Platform::kPlatformInfinite)
				{
					semaphore->available.wait(lock, [semaphore]
											  { return semaphore->count > 0; });
				}
				else if (!semaphore->available.wait_for(lock, std::chrono::milliseconds(timeoutMilliseconds),
														[semaphore]
														{ return semaphore->count > 0; }))
				{
					return false;
				}

				--semaphore->count;
				return true;
			}

			void CloseSemaphore(PlatformSemaphoreHandle handle) override
			{
				delete static_cast<PosixSemaphore*>(handle);
			}

			bool IsLinux() override
			{
				return true;
			}

			int CompilerVersion() override
			{
				return 1900;
			}

			void SetFloatingPointPrecision() override
			{
			}
	};
}

namespace Neoeter
{
	namespace Platform
	{
		IPlatformPort* CreatePlatformPort()
		{
			return new PosixPort();
		}
	}
}
