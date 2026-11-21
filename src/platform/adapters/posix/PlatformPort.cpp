#include <Neoeter/Platform/Port.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <thread>

#include <glob.h>
#include <unistd.h>

namespace
{
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
