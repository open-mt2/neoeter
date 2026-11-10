#include <support/TestAssert.h>

#include <Neoeter/Platform/Base/Version.h>

#include <cstring>
#include <string>

int main()
{
	TEST_SECTION("Neoeter version");

	const char* version = Neoeter::GetVersionString();
	TEST_ASSERT(version != nullptr);

	std::string text(version);
	TEST_ASSERT(!text.empty());
	TEST_ASSERT(text == NEOETER_VERSION_STRING);
	TEST_ASSERT(text.find_first_of("0123456789") != std::string::npos);

	return 0;
}
