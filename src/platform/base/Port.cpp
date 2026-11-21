#include <Neoeter/Platform/Port.h>

#include <memory>

namespace
{
	Neoeter::Platform::IPlatformPort*& OverridePort()
	{
		static Neoeter::Platform::IPlatformPort* s_port = nullptr;
		return s_port;
	}

	Neoeter::Platform::IPlatformPort& DefaultPort()
	{
		static std::unique_ptr<Neoeter::Platform::IPlatformPort> s_port(Neoeter::Platform::CreatePlatformPort());
		return *s_port;
	}
}

namespace Neoeter
{
	namespace Platform
	{
		IPlatformPort& Platform_GetPort()
		{
			IPlatformPort* port = OverridePort();
			return port ? *port : DefaultPort();
		}

		void Platform_SetPort(IPlatformPort* port)
		{
			OverridePort() = port;
		}
	}
}
