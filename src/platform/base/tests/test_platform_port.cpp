#include <support/TestAssert.h>

#include <Neoeter/Platform/Port.h>

#include <cstring>
#include <string>

namespace
{
	class FakePort final : public Neoeter::Platform::IPlatformPort
	{
		public:
			uint64_t MonotonicMillis() override { return 1234; }
			void SleepMillis(uint32_t milliseconds) override { slept = milliseconds; }
			uint32_t TickCount() override { return 99; }

			bool SetEnvironment(const char* name, const char* value) override
			{
				environmentName = name != nullptr ? name : "";
				environmentValue = value != nullptr ? value : "";
				return true;
			}

			bool FileExists(const char* path) override
			{
				lastPath = path != nullptr ? path : "";
				return lastPath == "present";
			}

			bool RemoveFile(const char* path) override
			{
				removedPath = path != nullptr ? path : "";
				return true;
			}

			bool CreateDirectory(const char* path) override
			{
				createdPath = path != nullptr ? path : "";
				return true;
			}

			std::string CurrentDirectory() override { return "/fake/dir"; }

			std::vector<std::string> FindFiles(const char* pattern) override
			{
				findPattern = pattern != nullptr ? pattern : "";
				return {findPattern, "second"};
			}

			bool RaiseTimerResolution(uint32_t* periodMilliseconds) override
			{
				if (periodMilliseconds != nullptr)
					*periodMilliseconds = 5;
				return true;
			}

			void RestoreTimerResolution(uint32_t periodMilliseconds) override { restoredPeriod = periodMilliseconds; }

			void ShowMessage(const char* title, const char* text) override
			{
				messageTitle = title != nullptr ? title : "";
				messageText = text != nullptr ? text : "";
			}

			int ToWide(const char* text, wchar_t* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity < 2)
					return 0;
				out[0] = static_cast<wchar_t>(text[0]);
				out[1] = L'\0';
				return 1;
			}

			int FromWide(const wchar_t* text, char* out, int capacity) override
			{
				if (text == nullptr || out == nullptr || capacity < 2)
					return 0;
				out[0] = static_cast<char>(text[0]);
				out[1] = '\0';
				return 1;
			}

			bool InitializeCom() override
			{
				comInitialized = true;
				return true;
			}

			void ShutdownCom() override { comInitialized = false; }

			PlatformModule ModuleHandle() override { return reinterpret_cast<PlatformModule>(0x1); }

			PlatformThreadHandle StartThread(uint32_t (*)(void*), void*) override { return nullptr; }
			bool JoinThread(PlatformThreadHandle, uint32_t) override { return true; }
			void CloseThread(PlatformThreadHandle) override {}
			PlatformSemaphoreHandle CreateSemaphore(uint32_t, uint32_t) override { return nullptr; }
			bool ReleaseSemaphore(PlatformSemaphoreHandle, uint32_t) override { return true; }
			bool WaitSemaphore(PlatformSemaphoreHandle, uint32_t) override { return false; }
			void CloseSemaphore(PlatformSemaphoreHandle) override {}

			bool IsLinux() override { return true; }
			int CompilerVersion() override { return 1900; }
			void SetFloatingPointPrecision() override { precisionSet = true; }

			uint32_t slept = 0;
			uint32_t restoredPeriod = 0;
			bool comInitialized = false;
			bool precisionSet = false;
			std::string environmentName;
			std::string environmentValue;
			std::string lastPath;
			std::string removedPath;
			std::string createdPath;
			std::string findPattern;
			std::string messageTitle;
			std::string messageText;
	};
}

int main()
{
	TEST_SECTION("Platform port forwarding");

	FakePort fake;
	Neoeter::Platform::Platform_SetPort(&fake);

	TEST_ASSERT(Neoeter::Platform::MonotonicMillis() == 1234);

	Neoeter::Platform::SleepMillis(17);
	TEST_ASSERT(fake.slept == 17);
	TEST_ASSERT(Neoeter::Platform::TickCount() == 99);

	TEST_ASSERT(Neoeter::Platform::SetEnvironment("MT2_TEST", "value"));
	TEST_ASSERT(fake.environmentName == "MT2_TEST");
	TEST_ASSERT(fake.environmentValue == "value");

	TEST_ASSERT(Neoeter::Platform::FileExists("present"));
	TEST_ASSERT(!Neoeter::Platform::FileExists("absent"));

	TEST_ASSERT(Neoeter::Platform::RemoveFile("old.tmp"));
	TEST_ASSERT(fake.removedPath == "old.tmp");

	TEST_ASSERT(Neoeter::Platform::CreateDirectory("mark"));
	TEST_ASSERT(fake.createdPath == "mark");
	TEST_ASSERT(Neoeter::Platform::CurrentDirectory() == "/fake/dir");

	const std::vector<std::string> files = Neoeter::Platform::FindFiles("BGM/*.mp3");
	TEST_ASSERT(files.size() == 2);
	TEST_ASSERT(files[0] == "BGM/*.mp3");
	TEST_ASSERT(fake.findPattern == "BGM/*.mp3");

	uint32_t period = 0;
	TEST_ASSERT(Neoeter::Platform::RaiseTimerResolution(&period));
	TEST_ASSERT(period == 5);
	Neoeter::Platform::RestoreTimerResolution(period);
	TEST_ASSERT(fake.restoredPeriod == 5);

	Neoeter::Platform::ShowMessage("title", "text");
	TEST_ASSERT(fake.messageTitle == "title");
	TEST_ASSERT(fake.messageText == "text");

	wchar_t wide[4] = {};
	TEST_ASSERT(Neoeter::Platform::ToWide("x", wide, 4) == 1);
	TEST_ASSERT(wide[0] == L'x');

	char narrow[4] = {};
	TEST_ASSERT(Neoeter::Platform::FromWide(L"y", narrow, 4) == 1);
	TEST_ASSERT(narrow[0] == 'y');

	TEST_ASSERT(Neoeter::Platform::InitializeCom());
	TEST_ASSERT(fake.comInitialized);
	Neoeter::Platform::ShutdownCom();
	TEST_ASSERT(!fake.comInitialized);

	TEST_ASSERT(Neoeter::Platform::ModuleHandle() != nullptr);
	TEST_ASSERT(Neoeter::Platform::IsLinux());
	TEST_ASSERT(Neoeter::Platform::CompilerVersion() == 1900);
	Neoeter::Platform::SetFloatingPointPrecision();
	TEST_ASSERT(fake.precisionSet);

	Neoeter::Platform::Platform_SetPort(nullptr);
	TEST_ASSERT(Neoeter::Platform::MonotonicMillis() != 1234);

	printf("[TEST] Platform port tests PASSED.\n");
	return 0;
}
